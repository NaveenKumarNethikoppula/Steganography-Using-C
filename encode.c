#include <stdio.h>
#include "encode.h"
#include "types.h"
#include "common.h"
#include<string.h>

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
Status copy_bmp_header(FILE *fptr_src_image,FILE *fptr_stego_image)
{
    char str[100];
    rewind(fptr_src_image);
    fread(str,54,1,fptr_src_image);
    fwrite(str,54,1,fptr_stego_image);
    return e_success;
}
// checking operation type for encoding or decoding
OperationType check_operation_type(int argc,char*argv[])
{
    if(argv[1][1] =='e')
    {
	return e_encode;
    }
    else if(argv[1][1] == 'd')
    {
	return e_decode;
    }
    else
    {
	return e_unsupported;
    }
}
// encoding magic string to the image file
Status encode_magic_string(char* magic_string,EncodeInfo *encInfo)
{
    int i;
    char arr[8];
    for(i=0;magic_string[i] != '\0';i++)
    {
	//count++;
	fread(arr,8,1,encInfo->fptr_src_image);
	encode_byte_to_lsb(magic_string[i],arr);
	fwrite(arr,8,1,encInfo->fptr_stego_image);
    }
    return e_success;
}
// encoding data the 1byte of data to  the 8byte of image data
Status encode_byte_to_lsb(char data, char* buffer_image)
{
    int j=0,i;
    for(i=7;i>=0;i--)
    {
	buffer_image[j] = ((data & (1<<i)) >> i) | (buffer_image[j] & (~1));
	j++;
    }
    return e_success;
}
// encoding the size of extension of file
Status encode_secret_file_extn_size(int size,EncodeInfo *encInfo)
{
    char str[32];
    fread(str,32,1,encInfo->fptr_src_image);
    encode_size_to_lsb(size,str);
    fwrite(str,32,1,encInfo->fptr_stego_image);
    return e_success;
}
// encoding the size 1byte of data to the 8bytes of output image data
Status encode_size_to_lsb(int data, char* buffer_image)
{
    int j=0,i=0;
    for(i=31;i>=0;i--)
    {
	buffer_image[j++] = ((data & (1<<i)) >> i) | (buffer_image[j] & (~1));
    }
    return e_success;
} 
// encoding data the 1byte of data to  the 8byte of output image data
Status encode_secret_file_extn(char *file_extn, EncodeInfo *encInfo)
{
    char str[8];
    for(int i=0;file_extn[i]!=0;i++)
    {
	fread(str,8,1,encInfo->fptr_src_image);
	encode_byte_to_lsb(file_extn[i],str);
	fwrite(str,8,1,encInfo->fptr_stego_image);
    }
}
// finding the size of file which contain secret info
uint get_file_size(FILE *fptr)
{
    fseek(fptr,0,SEEK_END);
    int len = ftell(fptr);
    return len;
}
// encoding the size of file which contain secret info
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char str[32];
    fread(str,32,1,encInfo->fptr_src_image);
    encode_size_to_lsb(file_size,str);
    fwrite(str,32,1,encInfo->fptr_stego_image);
}
// encoding the secret file data in to ouput image data
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    rewind(encInfo->fptr_secret);
    char ch[encInfo->size_secret_file],str[8];
    fread(ch,encInfo->size_secret_file,1,encInfo->fptr_secret);
    for(int i=0;i<encInfo->size_secret_file;i++)
    {
	fread(str,8,1,encInfo->fptr_src_image);
	encode_byte_to_lsb(ch[i],str);
	fwrite(str,8,1,encInfo->fptr_stego_image);
    }
    return e_success;
}
//copying the remaing data up to EOF
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    int ch;
    while ((ch = fgetc(fptr_src)) != EOF)
    {
	fputc(ch,fptr_dest);
    } 
}
//checking the output image capacity for storing it
Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    strcpy(encInfo->extn_secret_file,strstr(encInfo->secret_fname,"."));
    int size = strlen(encInfo->extn_secret_file);
    encInfo->size_secret_file=get_file_size(encInfo->fptr_secret);
    uint img_size;
    img_size = get_image_size_for_bmp(encInfo->fptr_src_image);
    printf("INFO : Image size = %u\n",img_size);
    if(encInfo->image_capacity > 54+40+32+(size*8)+32+(encInfo->size_secret_file*8))
    {
	return e_success;
    }
    else
    {
	return e_failure;
    }
}
// getting the image size for bmp file like width,height
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}
//start encoding process 
Status do_encoding(EncodeInfo *encInfo)
{
    if (open_files(encInfo) == e_success)
    {
	printf("SUCCESS: %s function completed\n", "open_files" );
	if(check_capacity(encInfo) == e_success)
	{
	    if(copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image)== e_success)
	    {
		printf("success");
		// call the function -> encode magic string
		encode_magic_string(MAGIC_STRING,encInfo);

		// store secret file extn into extn_secret_file array 
		strcpy(encInfo->extn_secret_file,strstr(encInfo->secret_fname,"."));
		int size=strlen(encInfo->extn_secret_file);
		encode_secret_file_extn_size(size,encInfo);

		// store secrete file extension
		encode_secret_file_extn(encInfo->extn_secret_file, encInfo);

		/* Get file size */
		//encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

		/* Encode secret file size */
		encode_secret_file_size(encInfo->size_secret_file, encInfo);

		/* Encode secret file data*/
		encode_secret_file_data(encInfo);
		// remaining data
		copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image);
		return e_success;
	    }
	    else
	    {	
		printf("Failure");
		return e_failure;
	    }
	}
    }
    else
    {
	printf("FAILED : %s function failure\n","open_files");
    }
}
// validating the encoding arguments  which were passing and calling functions
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    char arr[5];
    strcpy(arr,strstr(argv[2],"."));
    if((strcmp(arr,".bmp")==0))
    {
	encInfo->src_image_fname = argv[2];                                                        // storing the bmp file name
    }
    else
    {
	return e_failure;
    }
    if(argv[3] != NULL)
    {
	strcpy(arr,strstr(argv[3], "."));                                                             // store the secret file name  after comparing file extension
	if((strcmp(arr,".txt")==0)|| (strcmp(arr,".sh")==0) || (strcmp(arr,".c")==0))
	{
	    encInfo->secret_fname = argv[3];
	}
	else
	{
	    return e_failure;
	}
    }
    else
    {
	printf("Please Enter secret file \n");                                                     // show error message to user
	return e_failure;
    }
    if(argv[4]!=NULL)                                                                             // checking and storing the output file name which stores encoded data
    {
	strcpy(arr,strstr(argv[4],"."));
	if((strcmp(arr,".bmp")==0))
	{
	    encInfo->stego_image_fname = argv[4];
	}
	else
	{
	    return e_failure;
	}
    }
    else
    {
	encInfo->stego_image_fname="stego_image.bmp";                                        // default name for output file
    }
    return e_success;
}
/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    printf("enter\n");
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    printf("The File name is  %s\n",encInfo->src_image_fname);
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
	perror("fopen");
	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
	perror("fopen");
	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);
	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
	perror("fopen");
	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

	return e_failure;
    }

    // No failure return e_success
    return e_success;

}

