sources = [
	'source/main.cpp',
	'source/Initializer.cpp',
	'source/DebugHelper.cpp',
	'source/ConfigurationManagement/Configurator.cpp',
	'source/Threads/DefenseThread.cpp',
	'source/Cli.cpp',
	#'source/Threads/StatisticsThread.cpp',
	]

# Attacker
sources_attack = [
	'source/Attacker/main.cpp'
]

foreach string : sources
	if string != 'source/main.cpp'
		sources_attack += [string]
	endif
endforeach

