{
	"targets": [{
		"target_name":  "unix-socketpair",
		"sources":      ["src/socketpair.cpp", "src/util.cpp"],
		"cflags":       ["-std=c++11", "-Wall", "-Wextra", "-Wpedantic", "-Wunused-parameter"]
	}]
}
