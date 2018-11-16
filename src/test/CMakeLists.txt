add_executable(tests
	"../colors.cpp"
	"../diffparser.cpp"
	"../neonapp.cpp"
	"input.cpp")
catch_discover_tests(tests)
