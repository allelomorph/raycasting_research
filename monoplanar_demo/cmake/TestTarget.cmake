# TBD add list params to handle all <LANG> and <CONFIG> variations (plus VS refnames, globals, and xcode attributes?)
# https://cmake.org/cmake/help/v3.16/manual/cmake-properties.7.html#properties-on-targets
function(test_target target)
  if(TARGET ${target})
    message("${target} PROPERTIES")
    foreach(prop
        ADDITIONAL_CLEAN_FILES
        ALIASED_TARGET
        ANDROID_ANT_ADDITIONAL_OPTIONS
        ANDROID_API
        ANDROID_API_MIN
        ANDROID_ARCH
        ANDROID_ASSETS_DIRECTORIES
        ANDROID_GUI
        ANDROID_JAR_DEPENDENCIES
        ANDROID_JAR_DIRECTORIES
        ANDROID_JAVA_SOURCE_DIR
        ANDROID_NATIVE_LIB_DEPENDENCIES
        ANDROID_NATIVE_LIB_DIRECTORIES
        ANDROID_PROCESS_MAX
        ANDROID_PROGUARD
        ANDROID_PROGUARD_CONFIG_PATH
        ANDROID_SECURE_PROPS_PATH
        ANDROID_SKIP_ANT_STEP
        ANDROID_STL_TYPE
        #ARCHIVE_OUTPUT_DIRECTORY_<CONFIG>
        ARCHIVE_OUTPUT_DIRECTORY
        #ARCHIVE_OUTPUT_NAME_<CONFIG>
        ARCHIVE_OUTPUT_NAME
        AUTOGEN_BUILD_DIR
        AUTOGEN_ORIGIN_DEPENDS
        AUTOGEN_PARALLEL
        AUTOGEN_TARGET_DEPENDS
        AUTOMOC_COMPILER_PREDEFINES
        AUTOMOC_DEPEND_FILTERS
        AUTOMOC_EXECUTABLE
        AUTOMOC_FUNCTION_NAMES
        AUTOMOC_MOC_OPTIONS
        AUTOMOC_PATH_PREFIX
        AUTOMOC
        AUTOUIC
        AUTOUIC_EXECUTABLE
        AUTOUIC_OPTIONS
        AUTOUIC_SEARCH_PATHS
        AUTORCC
        AUTORCC_EXECUTABLE
        AUTORCC_OPTIONS
        BINARY_DIR
        BUILD_RPATH
        BUILD_RPATH_USE_ORIGIN
        BUILD_WITH_INSTALL_NAME_DIR
        BUILD_WITH_INSTALL_RPATH
        BUNDLE_EXTENSION
        BUNDLE
        C_EXTENSIONS
        C_STANDARD
        C_STANDARD_REQUIRED
        COMMON_LANGUAGE_RUNTIME
        COMPATIBLE_INTERFACE_BOOL
        COMPATIBLE_INTERFACE_NUMBER_MAX
        COMPATIBLE_INTERFACE_NUMBER_MIN
        COMPATIBLE_INTERFACE_STRING
        COMPILE_DEFINITIONS
        COMPILE_FEATURES
        COMPILE_FLAGS
        COMPILE_OPTIONS
        COMPILE_PDB_NAME
        #COMPILE_PDB_NAME_<CONFIG>
        COMPILE_PDB_OUTPUT_DIRECTORY
        #COMPILE_PDB_OUTPUT_DIRECTORY_<CONFIG>
        #<CONFIG>_OUTPUT_NAME
        #<CONFIG>_POSTFIX
        CROSSCOMPILING_EMULATOR
        CUDA_PTX_COMPILATION
        CUDA_SEPARABLE_COMPILATION
        CUDA_RESOLVE_DEVICE_SYMBOLS
        CUDA_EXTENSIONS
        CUDA_STANDARD
        CUDA_STANDARD_REQUIRED
        CXX_EXTENSIONS
        CXX_STANDARD
        CXX_STANDARD_REQUIRED
        CXX_CLANG_TIDY           #<LANG>_CLANG_TIDY
        CXX_COMPILER_LAUNCHER    #<LANG>_COMPILER_LAUNCHER
        CXX_CPPCHECK             #<LANG>_CPPCHECK
        CXX_CPPLINT              #<LANG>_CPPLINT
        CXX_INCLUDE_WHAT_YOU_USE #<LANG>_INCLUDE_WHAT_YOU_USE
        CXX_VISIBILITY_PRESET    #<LANG>_VISIBILITY_PRESET
        DEBUG_POSTFIX
        DEFINE_SYMBOL
        DEPLOYMENT_REMOTE_DIRECTORY
        DEPLOYMENT_ADDITIONAL_FILES
        DISABLE_PRECOMPILE_HEADERS
        DOTNET_TARGET_FRAMEWORK_VERSION
        EchoString
        ENABLE_EXPORTS
        EXCLUDE_FROM_ALL
        #EXCLUDE_FROM_DEFAULT_BUILD_<CONFIG>
        EXCLUDE_FROM_DEFAULT_BUILD
        EXPORT_NAME
        EXPORT_PROPERTIES
        FOLDER
        Fortran_FORMAT
        Fortran_MODULE_DIRECTORY
        FRAMEWORK
        FRAMEWORK_VERSION
        GENERATOR_FILE_NAME
        GHS_INTEGRITY_APP
        GHS_NO_SOURCE_GROUP_FILE
        GNUtoMS
        HAS_CXX
        IMPLICIT_DEPENDS_INCLUDE_TRANSFORM
        IMPORTED_COMMON_LANGUAGE_RUNTIME
        IMPORTED_CONFIGURATIONS
        IMPORTED_GLOBAL
        #IMPORTED_IMPLIB_<CONFIG>
        IMPORTED_IMPLIB
        #IMPORTED_LIBNAME_<CONFIG>
        IMPORTED_LIBNAME
        #IMPORTED_LINK_DEPENDENT_LIBRARIES_<CONFIG>
        IMPORTED_LINK_DEPENDENT_LIBRARIES
        #IMPORTED_LINK_INTERFACE_LANGUAGES_<CONFIG>
        IMPORTED_LINK_INTERFACE_LANGUAGES
        #IMPORTED_LINK_INTERFACE_LIBRARIES_<CONFIG>
        IMPORTED_LINK_INTERFACE_LIBRARIES
        #IMPORTED_LINK_INTERFACE_MULTIPLICITY_<CONFIG>
        IMPORTED_LINK_INTERFACE_MULTIPLICITY
        #IMPORTED_LOCATION_<CONFIG>
        IMPORTED_LOCATION
        #IMPORTED_NO_SONAME_<CONFIG>
        IMPORTED_NO_SONAME
        #IMPORTED_OBJECTS_<CONFIG>
        IMPORTED_OBJECTS
        IMPORTED
        #IMPORTED_SONAME_<CONFIG>
        IMPORTED_SONAME
        IMPORT_PREFIX
        IMPORT_SUFFIX
        INCLUDE_DIRECTORIES
        INSTALL_NAME_DIR
        INSTALL_REMOVE_ENVIRONMENT_RPATH
        INSTALL_RPATH
        INSTALL_RPATH_USE_LINK_PATH
        INTERFACE_AUTOUIC_OPTIONS
        INTERFACE_COMPILE_DEFINITIONS
        INTERFACE_COMPILE_FEATURES
        INTERFACE_COMPILE_OPTIONS
        INTERFACE_INCLUDE_DIRECTORIES
        INTERFACE_LINK_DEPENDS
        INTERFACE_LINK_DIRECTORIES
        INTERFACE_LINK_LIBRARIES
        INTERFACE_LINK_OPTIONS
        INTERFACE_PRECOMPILE_HEADERS
        INTERFACE_POSITION_INDEPENDENT_CODE
        INTERFACE_SOURCES
        INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
        #INTERPROCEDURAL_OPTIMIZATION_<CONFIG>
        INTERPROCEDURAL_OPTIMIZATION
        IOS_INSTALL_COMBINED
        JOB_POOL_COMPILE
        JOB_POOL_LINK
        LABELS
        #<LANG>_CLANG_TIDY
        #<LANG>_COMPILER_LAUNCHER
        #<LANG>_CPPCHECK
        #<LANG>_CPPLINT
        #<LANG>_INCLUDE_WHAT_YOU_USE
        #<LANG>_VISIBILITY_PRESET
        #LIBRARY_OUTPUT_DIRECTORY_<CONFIG>
        LIBRARY_OUTPUT_DIRECTORY
        #LIBRARY_OUTPUT_NAME_<CONFIG>
        LIBRARY_OUTPUT_NAME
        LINK_DEPENDS_NO_SHARED
        LINK_DEPENDS
        LINKER_LANGUAGE
        LINK_DIRECTORIES
        #LINK_FLAGS_<CONFIG>
        LINK_FLAGS
        #LINK_INTERFACE_LIBRARIES_<CONFIG>
        LINK_INTERFACE_LIBRARIES
        #LINK_INTERFACE_MULTIPLICITY_<CONFIG>
        LINK_INTERFACE_MULTIPLICITY
        LINK_LIBRARIES
        LINK_OPTIONS
        LINK_SEARCH_END_STATIC
        LINK_SEARCH_START_STATIC
        LINK_WHAT_YOU_USE
        #LOCATION_<CONFIG>
        LOCATION
        MACOSX_BUNDLE_INFO_PLIST
        MACOSX_BUNDLE
        MACOSX_FRAMEWORK_INFO_PLIST
        MACOSX_RPATH
        MANUALLY_ADDED_DEPENDENCIES
        #MAP_IMPORTED_CONFIG_<CONFIG>
        MSVC_RUNTIME_LIBRARY
        NAME
        NO_SONAME
        NO_SYSTEM_FROM_IMPORTED
        OBJC_EXTENSIONS
        OBJC_STANDARD
        OBJC_STANDARD_REQUIRED
        OBJCXX_EXTENSIONS
        OBJCXX_STANDARD
        OBJCXX_STANDARD_REQUIRED
        #OSX_ARCHITECTURES_<CONFIG>
        OSX_ARCHITECTURES
        #OUTPUT_NAME_<CONFIG>
        OUTPUT_NAME
        #PDB_NAME_<CONFIG>
        PDB_NAME
        #PDB_OUTPUT_DIRECTORY_<CONFIG>
        PDB_OUTPUT_DIRECTORY
        POSITION_INDEPENDENT_CODE
        PRECOMPILE_HEADERS
        PRECOMPILE_HEADERS_REUSE_FROM
        PREFIX
        PRIVATE_HEADER
        PROJECT_LABEL
        PUBLIC_HEADER
        RESOURCE
        RULE_LAUNCH_COMPILE
        RULE_LAUNCH_CUSTOM
        RULE_LAUNCH_LINK
        #RUNTIME_OUTPUT_DIRECTORY_<CONFIG>
        RUNTIME_OUTPUT_DIRECTORY
        #RUNTIME_OUTPUT_NAME_<CONFIG>
        RUNTIME_OUTPUT_NAME
        SKIP_BUILD_RPATH
        SOURCE_DIR
        SOURCES
        SOVERSION
        #STATIC_LIBRARY_FLAGS_<CONFIG>
        STATIC_LIBRARY_FLAGS
        STATIC_LIBRARY_OPTIONS
        SUFFIX
        Swift_DEPENDENCIES_FILE
        Swift_LANGUAGE_VERSION
        Swift_MODULE_DIRECTORY
        Swift_MODULE_NAME
        TYPE
        UNITY_BUILD
        UNITY_BUILD_BATCH_SIZE
        UNITY_BUILD_CODE_AFTER_INCLUDE
        UNITY_BUILD_CODE_BEFORE_INCLUDE
        VERSION
        VISIBILITY_INLINES_HIDDEN
        VS_CONFIGURATION_TYPE
        VS_DEBUGGER_COMMAND
        VS_DEBUGGER_COMMAND_ARGUMENTS
        VS_DEBUGGER_ENVIRONMENT
        VS_DEBUGGER_WORKING_DIRECTORY
        VS_DESKTOP_EXTENSIONS_VERSION
        #VS_DOTNET_REFERENCE_<refname>
        #VS_DOTNET_REFERENCEPROP_<refname>_TAG_<tagname>
        VS_DOTNET_REFERENCES
        VS_DOTNET_REFERENCES_COPY_LOCAL
        VS_DOTNET_TARGET_FRAMEWORK_VERSION
        VS_DPI_AWARE
        VS_GLOBAL_KEYWORD
        VS_GLOBAL_PROJECT_TYPES
        VS_GLOBAL_ROOTNAMESPACE
        #VS_GLOBAL_<variable>
        VS_IOT_EXTENSIONS_VERSION
        VS_IOT_STARTUP_TASK
        VS_JUST_MY_CODE_DEBUGGING
        VS_KEYWORD
        VS_MOBILE_EXTENSIONS_VERSION
        VS_NO_SOLUTION_DEPLOY
        VS_PACKAGE_REFERENCES
        VS_PROJECT_IMPORT
        VS_SCC_AUXPATH
        VS_SCC_LOCALPATH
        VS_SCC_PROJECTNAME
        VS_SCC_PROVIDER
        VS_SDK_REFERENCES
        VS_USER_PROPS
        VS_WINDOWS_TARGET_PLATFORM_MIN_VERSION
        VS_WINRT_COMPONENT
        VS_WINRT_EXTENSIONS
        VS_WINRT_REFERENCES
        WIN32_EXECUTABLE
        WINDOWS_EXPORT_ALL_SYMBOLS
        #XCODE_ATTRIBUTE_<an-attribute>
        XCODE_EXPLICIT_FILE_TYPE
        XCODE_GENERATE_SCHEME
        XCODE_PRODUCT_TYPE
        XCODE_SCHEME_ADDRESS_SANITIZER
        XCODE_SCHEME_ADDRESS_SANITIZER_USE_AFTER_RETURN
        XCODE_SCHEME_ARGUMENTS
        XCODE_SCHEME_DEBUG_AS_ROOT
        XCODE_SCHEME_DEBUG_DOCUMENT_VERSIONING
        XCODE_SCHEME_DISABLE_MAIN_THREAD_CHECKER
        XCODE_SCHEME_DYNAMIC_LIBRARY_LOADS
        XCODE_SCHEME_DYNAMIC_LINKER_API_USAGE
        XCODE_SCHEME_ENVIRONMENT
        XCODE_SCHEME_EXECUTABLE
        XCODE_SCHEME_GUARD_MALLOC
        XCODE_SCHEME_MAIN_THREAD_CHECKER_STOP
        XCODE_SCHEME_MALLOC_GUARD_EDGES
        XCODE_SCHEME_MALLOC_SCRIBBLE
        XCODE_SCHEME_MALLOC_STACK
        XCODE_SCHEME_THREAD_SANITIZER
        XCODE_SCHEME_THREAD_SANITIZER_STOP
        XCODE_SCHEME_UNDEFINED_BEHAVIOUR_SANITIZER
        XCODE_SCHEME_UNDEFINED_BEHAVIOUR_SANITIZER_STOP
        XCODE_SCHEME_ZOMBIE_OBJECTS
        XCTEST
        )
      get_target_property(val ${target} ${prop})
      # filter out ${val}-NOTFOUND
      if(val)
        message("    ${prop}: ${val}")
      endif()
    endforeach()
  endif()
endfunction()
