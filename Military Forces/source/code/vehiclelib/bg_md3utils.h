#ifndef __BG_MD3UTILS_H__
#define __BG_MD3UTILS_H__

#include <string>
#include <vector>





struct Md3Utils
{
	// helper enum to retrieve size of model 
	enum
	{
		FRAMESIZE_ALL		= -1,
		FRAMESIZE_FIRST		= -2,
		FRAMESIZE_LAST		= -3
	};

	// retrieve the size of a model from a file.
	// if "frame" is specified it gets the dimensions of this frame, has to be a number from
	// 0 to maxframe, or one of the FRAMESIZE_* enums
	static bool				getModelDimensions( std::string const& filename,
												vec3_t& mins,
												vec3_t& maxs,
												int checkFrame = FRAMESIZE_ALL );

	// find all tags containing string str
	static int				getTagsContaining( std::string const& filename, 
											   std::string const& str,
											   std::vector<md3Tag_t>& tagList );

private:
	// pure utils class
						Md3Utils();
						~Md3Utils();
};		







#endif	// __BG_MD3UTILS_H__