project('Aegis', 'cpp',
	# Get version number from file.
	# Fallback to "more" for Windows compatibility.
	version: run_command(find_program('cat', 'more'),
		files('VERSION')).stdout().strip(),
	license: 'BSD',
	default_options: ['cpp_std=c++17', 'buildtype=debug', 'default_library=static', ],
	meson_version: '>= 0.58.0'
)

# check the OS is supported, rather than going any further
supported_exec_envs = ['linux']
exec_env = host_machine.system()
if not supported_exec_envs.contains(exec_env)
	error('unsupported system type "@0@"'.format(exec_env))
endif

# set up some global vars for compiler, platform, configuration, etc.
compiler = meson.get_compiler('cpp')
compiler_version = compiler.version()

# set include directory (where the header files live)
inc = include_directories('include')

#project_libary = libary('project',files[sources])

# find dependencies
dpdk_dep = dependency('libdpdk', required : false)
project_dep = declare_dependency(include_directories : inc)
pthread_dep = dependency('threads')
boost_dep = dependency('boost', modules: ['filesystem','system','log'])

if boost_dep.found()
    message('boost found')

    message('building on: "' + exec_env + '"')
    message('with compiler: "' + compiler.version() + '"')
    message('host machine: "' + host_machine.system() + '"')

    subdir('doc')
    subdir('source')

    if dpdk_dep.found()
        executable(
	        'aegis',
	        sources,
    	    include_directories : inc,
    	    dependencies : [dpdk_dep,pthread_dep,boost_dep]
    	    )
        
# =============   A T T A C K E R   ================= #

        executable(
	        'syntflut',
	        sources_attack,
    	    include_directories : inc,
    	    dependencies : [dpdk_dep,pthread_dep,boost_dep],
            cpp_args : '-DATTACK'
    	    )
    endif


#================   T E S T S   ==================== #

    catch2_dep = dependency('catch2', 
                       fallback : ['catch2', 'catch2_dep'],
                       required: true)

    catch2 = declare_dependency(sources:'catch.cpp',dependencies:[catch2_dep])

    subdir('test')

    inc_test = include_directories('include', 'test/libdpdk_dummy/include')
    
    # create new string array which has the same elements like sources but without the source/main.cpp file
    sources_tests = []
    foreach string : sources
	    if (string != 'source/main.cpp') and (string != 'source/Initializer.cpp')
		    sources_tests += [string]
	    endif
    endforeach

    # create test targets
    foreach t : test_sources_dict.keys()
	    exe_name = t+'_test'
	    test_name = t+'_test'

	    sources_this_test = sources_tests + [test_sources_dict.get(t)]

	    e = executable(
            exe_name,
            sources_this_test,
            include_directories : inc_test,
            dependencies : [catch2, pthread_dep, boost_dep],
            cpp_args : '-DTEST'
        )
            
	    test(test_name, e, suite:['unit_tests', 'catch2'])
        message('built test ' + test_name)

    endforeach

    # this does not affect any part of the build, for information only.
    message('\n=================\nMeson prepared build\nrun Ninja to build\n=================\n')
endif

if not boost_dep.found() 
    warning('boost not found')
endif
if not dpdk_dep.found()
    warning('dpdk not found; only unit tests can be executed')
endif
