Generate and Encode data to JSON


Generating JSON
----------------
One can use parse_json to serialize and deserialize(less tested) their data.  

    #include <daw/json/daw_json_link.h>
    #include <iostream>
    #include <cstdlib>
    
    struct Test: public daw::json::JsonLink<Test> {
        int b;
        double c;
        Test( ):
                daw::json::JsonLink<Test>( "Test" ),
                b{ 0 },
                c{ 0.0 } {
            link_integral( "b", b );
            link_real( "c", c );
        }
    };
  
    int main( int, char ** ) {
      A a;
      a.b = 1234;
      a.c = 10.001;
      
      std::cout << = a.encode( );
      return EXIT_SUCCESS;
    }
    
would output:
    "Test": { "b": 1234, "c": 10.000999999999999 }

Parsing JSON
-------------
