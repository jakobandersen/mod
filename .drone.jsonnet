local image = "localhost:5000/jla/mod";
local boostArg(v) = "-DBOOST_ROOT=/opt/boost/%s" % v;

local CoverageStep(withCoverage, compiler, boost) = if !withCoverage then null else {
	name: "coverage",
	image: image,
	environment: {
		CTEST_OUTPUT_ON_FAILURE: 1,
		CXX: compiler,
		CXXFLAGS: "-Werror",
	},
	commands: [
		"mkdir covBuild",
		"cd covBuild",
		"cmake ../ -DCMAKE_BUILD_TYPE=OptDebug -DBUILD_TESTING=on -DBUILD_COVERAGE=on %s" % [boostArg(boost)],
		"make",
		"make install",
		"make tests",
		"make coverage_collect",
		"make coverage_build",
		"/copyCoverage.sh",
	],
	when: {
		ref: [
			"refs/heads/develop",
			"refs/heads/master",
			"refs/tags/v*",
		]
	},
	volumes: [
		{
			name: "www",
			path: "/www",
		},
	],
};

local Volumes(withCoverage) = if !withCoverage then [] else [
	{
		name: "www",
		host: {
			path: "/www/results/mod",
		},
	},
];

local Bootstrap(dep=false) = {
	name: "bootstrap",
	image: image,
	commands: [
		"git fetch --tags",
		"git submodule update --init --recursive",
		"./bootstrap.sh",
	],
	[ if dep then "depends_on"]: [ "clone" ],
};

local Configure(compiler, boost, dep=false) = {
	name: "configure",
	image: image,
	environment: {
		CXX: compiler,
		CXXFLAGS: "-Werror",
	},
	commands: [
		"mkdir build",
		"cd build",
		"cmake ../ -DCMAKE_BUILD_TYPE=OptDebug -DBUILD_DOC=on -DBUILD_TESTING=on -DBUILD_TESTING_SANITIZERS=off %s" % [boostArg(boost)],
	],
	[ if dep then "depends_on"]: [ "bootstrap" ],
};

local Pipeline(withCoverage, compiler, boost) = {
	kind: "pipeline",
	name: "%s, Boost %s" % [compiler, boost],
	steps: [
		Bootstrap(),
		Configure(compiler, boost),
		{
			name: "build",
			image: image,
			commands: [
				"cd build",
				"cmake ../ -DBUILD_TESTING=on -DBUILD_TESTING_SANITIZERS=off %s" % [boostArg(boost)],
				"make",
			],
		},
		{
			name: "install",
			image: image,
			commands: [
				"cd build",
				"make install",
			],
		},
		{
			name: "build-test",
			image: image,
			commands: [
				"cd build",
				"make tests",
			],
		},
		{
			name: "test",
			image: image,
			commands: [
				"cd build",
				"make install",
				"ctest --output-on-failure -E cmake_add_subdirectory_build",
			],
		},
		{
			name: "simple test",
			image: image,
			commands: [
				"cd build",
				"make install",
				"cd ../",
				"mod -e \"smiles('O').print()\"",
			],
		},
		{
			name: "test subdirectory build",
			image: image,
			commands: [
				"cd build",
				"make install",
				"ctest --output-on-failure -R cmake_add_subdirectory_build",
			],
			when: {
				ref: [
					"refs/heads/develop",
					"refs/heads/master",
					"refs/tags/v*",
				]
			},
		},
		CoverageStep(withCoverage, compiler, boost),
	],
	volumes: Volumes(withCoverage),
};

[
	{
		kind: "pipeline",
		name: "Various Static Checks",
		steps: [
			{
				name: "Python, mypy",
				image: image,
				commands: [
					"./scripts/mypy.sh",
				],
				depends_on: [ "clone" ],
			},
			{
				name: "Python, flake8",
				image: image,
				commands: [
					"./scripts/flake8.sh",
				],
				depends_on: [ "clone" ],
			},
			{
				name: "Symbol Visibility, json",
				image: image,
				commands: [
					"./scripts/checkJsonVisibility.sh",
				],
				depends_on: [ "clone" ],
			},
		]
	},
	{
		kind: "pipeline",
		name: "Docker",
		steps: [
			Bootstrap(false),
			Configure("g++", "1_75_0", false),
			{
				name: "dist",
				image: image,
				commands: [
					"cd build",
					"make dist",
				],
				//depends_on: [ "configure" ],
			},
			{
				name: "Ubuntu",
				image: "plugins/docker",
				settings: {
					repo: "jakobandersen/mod",
					tags: ["ubuntu-test"],
					dockerfile: "docker/Ubuntu.Dockerfile",
					dry_run: true,
					purge: true,
					build_args: ["j=1"],
				},
				//depends_on: [ "dist" ],
			},
			{
				name: "Fedora",
				image: "plugins/docker",
				settings: {
					repo: "jakobandersen/mod",
					tags: ["fedora-test"],
					dockerfile: "docker/Fedora.Dockerfile",
					dry_run: true,
					purge: true,
					build_args: ["j=1"],
				},
				//depends_on: [ "dist" ],
			},
			{
				name: "Arch",
				image: "plugins/docker",
				settings: {
					repo: "jakobandersen/mod",
					tags: ["arch-test"],
					dockerfile: "docker/Arch.Dockerfile",
					dry_run: true,
					purge: true,
					build_args: ["j=1"],
				},
				//depends_on: [ "dist" ],
			},
		]
	},
] + [
	Pipeline(boost == "1_74_0" && compiler == "g++-9", compiler, boost)
	for compiler in [
		"g++-8", "g++-9", "g++-10", "g++-11",
		"clang++-8", "clang++-9", "clang++-10", "clang++-11", "clang++-12",
	]
	for boost in [
		"1_73_0", "1_74_0", "1_75_0", "1_76_0",
	]
]
