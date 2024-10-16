#include "File.h"

void File::Open() {
	// make the sure the endiness is correct

	// make sure the file type is correct

	// make sure the version is compatible

	// make sure the size is not to big ??

	// make sure the header is not empty
	 
    // make sure the resolution is valid
	
	// uncompress the header

	// for every entry in the header load the tile as compressed from the data offset and length
}

void File::Save() {
	// Compress any non compressed tile

	// Regenerate the header

    // compress the header

	// Regenerate the data

}

void File::Close() {
}
