# list of all example apps. Keep 1-3 per line, in alphabetical order.
all_examples = [
	'receive', 'send'
]

if get_option('examples') == ''
	subdir_done()
endif

if get_option('examples').to_lower() == 'all'
	examples = all_examples
	allow_skips = true # don't flag an error if we can't build an app
else
	examples = get_option('examples').split(',')
	allow_skips = false # error out if we can't build a requested app
endif
default_cflags = machine_args
if cc.has_argument('-Wno-format-truncation')
	default_cflags += '-Wno-format-truncation'
endif
default_ldflags = dpdk_extra_ldflags
if get_option('default_library') == 'static' and not is_windows
	default_ldflags += ['-Wl,--export-dynamic']
endif

foreach example: examples
	name = example.split('/')[-1]
	build = true
	sources = []
	allow_experimental_apis = false
	cflags = default_cflags
	ldflags = default_ldflags

	ext_deps = [execinfo]
	includes = [include_directories(example)]
	deps = ['eal', 'mempool', 'net', 'mbuf', 'ethdev', 'cmdline']
	subdir(example)

	if build
		dep_objs = ext_deps
		foreach d:deps
			var_name = get_option('default_library') + '_rte_' + d
			if not is_variable(var_name)
				error('Missing dependency "@0@" for example "@1@"'.format(d, name))
			endif
			dep_objs += [get_variable(var_name)]
		endforeach
		if allow_experimental_apis
			cflags += '-DALLOW_EXPERIMENTAL_API'
		endif
		executable('dpdk-' + name, sources,
			include_directories: includes,
			link_whole: link_whole_libs,
			link_args: ldflags,
			c_args: cflags,
			dependencies: dep_objs)
	elif not allow_skips
		error('Cannot build requested example "' + name + '"')
	else
		message('Skipping example "' + name + '"')
	endif
endforeach
