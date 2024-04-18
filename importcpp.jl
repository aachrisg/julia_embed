using Base.Libc.Libdl

struct TypeReference_t
    m_pNext::Ptr{TypeReference_t}
    m_pTypeName::Ptr{Cchar}
    m_pUsageName::Ptr{Cchar}
end

struct FunctionDeclaration_t
    m_pName::Ptr{Cchar}
    m_functionPtr::Ptr{Nothing}
    m_pReturnType::Ptr{TypeReference_t}
    m_pArgTypes::Ptr{TypeReference_t}
end

struct SymbolTableList_t
    m_nNumSymbols::Int64
    m_pExposedSymbols::Ptr{FunctionDeclaration_t}
end

CPPTypeToJuliaType = Dict(
    "int" => Cint,
    "float" => Cfloat,
    "double" => Cdouble,
    "size_t" => Csize_t,
    "void" => Cvoid
)

# we want some function parameters to do type conversion such as double to float, etc
CPPTypeToJuliaGenericType = Dict(
    "int" => Integer,
    "float" => Number,
    "double" => Number,
    "size_t" => Integer,
)

function ImportCPP( dllName::AbstractString, cmdLineArgs::Vector{String}=Vector{String}( [] ) )
    dll = dlopen( dllName )
    # call Setup() in the DLL
    argArray = vcat( dllName, cmdLineArgs );
    ccall( dlsym( dll, :Setup ), Nothing, ( Cint, Ptr{Ptr{Cchar}} ), size( argArray, 1), argArray )
    
    # create the variable XXX_DLL
    varNameForDLL = Symbol( dllName * "_DLL" );
    @eval $varNameForDLL = $dll
    
    # create the UnloadXXX function
    unloadFnName = Symbol( basename( dllName ) * "_Unload" )
    println( "to unload, call ", unloadFnName )
    unloadFnBody = quote
        function $unloadFnName()
            #println( "Unloading DLL ", $dllName )
            dlclose( $varNameForDLL );
        end
    end
    
    # get the exposed symbol list
    exposures::SymbolTableList_t = ccall( dlsym( dll, :GetExposedSymbols ), SymbolTableList_t, () )
    for i in 1:exposures.m_nNumSymbols
        functionCantBeImported = false
        exposedFunction = unsafe_load( exposures.m_pExposedSymbols, i )
        exposedName = unsafe_string( exposedFunction.m_pName )
        exposedName = replace( exposedName, r"<.*" => "" );
        exposedFunctionPtr = exposedFunction.m_functionPtr

        cReturnType =unsafe_string( unsafe_load( exposedFunction.m_pReturnType ).m_pTypeName )
        juliaReturnType = get( CPPTypeToJuliaType, cReturnType, Nothing )
        if ( juliaReturnType == Nothing ) && ( cReturnType != "void" )
            println( "Unrecognized return type $cReturnType for function $exposedName" )
            continue
        end
        
        functionSymbol = Symbol( exposedName )
        varSym = Symbol( dllName * "_FunctionPointerTo" * String( exposedName ) )
        @eval $varSym = $exposedFunctionPtr
        funcdef = quote
            function $functionSymbol( )
                if $varSym == C_NULL
                    error( "Shared library ", $dllName, " has been unloaded for ", $exposedName )
                end
                ccall( $varSym, $juliaReturnType, () )
            end
        end
        # now, we will add the argument list to the function definiition
        argPtr = exposedFunction.m_pArgTypes #unsafe_load( exposedFunction.m_pArgTypes );
        argName = 'A'
        while( argPtr != C_NULL )
            cArgType = unsafe_string( unsafe_load( argPtr ).m_pTypeName );
            juliaArgType = get( CPPTypeToJuliaType, cArgType, Nothing )
            if juliaArgType == Nothing
                println( "Unrecognized argument type $cArgType for function $exposedName" )
                functionCantBeImported = true
                break
            end
            genericArgType = get( CPPTypeToJuliaGenericType, cArgType, juliaArgType )
            argNameSymbol = Symbol( "Arg" * argName )
            argName += 1
            argList = :( $argNameSymbol::$genericArgType )
            # add it to the function proto
            push!( funcdef.args[2].args[1].args, argList )
            # add it to the parameter types in the ccall
            push!( funcdef.args[2].args[2].args[5].args[4].args, juliaArgType )
            # add the passing of the arg
            push!( funcdef.args[2].args[2].args[5].args , argNameSymbol )
            argPtr = unsafe_load( argPtr ).m_pNext
        end
        eval( funcdef )
        push!( unloadFnBody.args[2].args[2].args, quote
                  # println( "unload ", $exposedName )
                  global $varSym = C_NULL;
               end )
    end
    eval( unloadFnBody );
end


