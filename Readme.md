#Parse Json

This is mainly a serialization/deserializatio library that allows your to easily use json as the transport medium.

```
#include <cstdlib>
#include <string>

#include <daw/json/daw_json_link.h>

struct config_t : public daw::json::json_link<config_t> {
	int port;
	std::string url_path;

	config_t( ) : daw::json::json_link<config_t>{}, port{8080}, url_path{"/"} {}

	config_t( config_t const &other ) = default;
	config_t( config_t &&other ) = default;
	config_t &operator=( config_t const & ) = default;
	config_t &operator=( config_t && ) = default;
	~config_t( ) = default;

	static void map_to_json( ) {
		json_link_integer( "port", port );
		json_link_string( "url_path", url_path );
	}
};

int main( int argc, char **argv ) {
	config_t config{};
	if( argc > 1 ) {
		config = daw::json::from_file<config_t>( argv[1] );
	} else {
		std::string path = argv[0] + ".json";
		daw::json::to_file<config_t>( path );
	}

	return EXIT_SUCCESS;
}
```

As you can see, the requirement is to inherit from the json_link class with the current class as a template parameter.  And then, use the appropriate json_link_... command to give it a name and a variable to link to.


The order of the object element names is sorted in lexigraphical order of the codepoints.  This may or may not make sense depending on the situation 
