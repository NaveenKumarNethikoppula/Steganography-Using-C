#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "common.h"
#include "types.h"
#include "decode.h"
/*
   receiving arguments from command line and validating
 */
int main( int argc ,char * argv[])
{
    if(argc ==1)                                                                // check condition for argument contains encoding or decoding
    {
	printf("please select encoding or decoding\n");
	return 0;
    }
    if((argc ==1 || argc ==2) && (argv[1][1] =='e'))                             // check the argument for passing -e or -d
    {
	printf("please pass : ./a.out -e beautiful.bmp secret.txt \n");
	return 0;
    }
    if((argc ==1 || argc ==2) && (argv[1][1] == 'd'))                            // check the argument for passing -e or -d
    {
	printf("please pass : ./a.out -d stego_img.bmp <output>\n");
	return 0;
    }
    if(argc > 2)
    {
	int ret = check_operation_type(argc,argv);                                  // store the check operation type i.e., e-encode or e_decode
	if(ret == e_encode)                                                        // if it is e_encode start encoding
	{
	    printf("Selected Encode\n");
	    EncodeInfo encInfo;
	    
	    if(read_and_validate_encode_args(argv, &encInfo) == e_success)         // read and validate encode args are e_success or e_failure
	    {
		printf("read_and_validation is successfully done\n");
		if( do_encoding(&encInfo) == e_success)
		{
		    printf("encoding is successfull done\n");
		}
		else
		{
		    printf("read_and_validation is Failure\n");
		}
	    }
	    else
	    {
		printf("read_and_validation is Failed\n");
	    }

	}
	else if ( ret == e_decode )                                                // check return is a decode or not
	{
	    printf("Selected Decoding\n");
	    DecodeInfo decInfo;
	    if(read_and_validate_decode_args(argc,argv,&decInfo) == e_success)     // check decode arguments return e-success or not
	    {
		printf("Read and validation for decode is successfull\n");
		if(do_decoding(&decInfo) == e_success)
		{
		    printf("decoding is successfull\n");
		}
		else
		{
		    printf("decoding failed\n");
		}
	    }
	    else
	    {
		printf("Failed to read and validate decode args\n");
	    }

	}
	else
	{
	    printf("unsupported\n");                                                // show error to user to enter proper process
	}
    }
    return 0;
}
