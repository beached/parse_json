#Parse Json

This is mainly a serialization/deserializatio library that allows your to easily use json as the transport medium.

```
#include <string>
#include <daw/json/daw_json_link.h>

struct config_t : public daw::json::JsonLink<config_t> {
	int port;
	std::string url_path;

	config_t( ):
			daw::json::JsonLink<config_t>{ },
			port{ 8080 },
			url_path{ "/" } {

		link_int( "port", port );
		link_string( "url_path", url_path );
	}
};

int main( int argc, char ** argv ) {
	config_t config{ };
	if( argc > 1 ) {
		config.decode_file( argv[1] );
	} else {
		std::string path = argv[0] + ".json";
		config.encode_file( path );
	}

	return 0;
}

```

As you can see, the requirement is to inherit from the JsonLink class with the current class as a template parameter.  And then, use the appropriate link_... command to give it a name and a varialbe to link to.


The order of the object element names is sorted in lexigraphical order of the codepoints.  This may or may not make sense depending on the situation 
