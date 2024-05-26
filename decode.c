#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decode.h"
#include "types.h"

Status read_and_validate_decode_args(int argc, char *argv[], DecodeInfo *decInfo)
{
    if(argc > 2) 									// if argument count 1 store ouput.bmp fime in a stegoimagefilenmae
    {
	if(strcmp(strstr(argv[2], "."),".bmp") == 0)
	{
	    decInfo->stego_image_fname = argv[2];
	}
	else
	{
	    fprintf(stderr,"Error : Stego image %s format should be .bmp\n", argv[2]); // show error messagemto user
	    return e_failure;
	}
    }
    else
    {
	fprintf(stderr,"Error : Arguments are misssing\n"); 		        	// show error message to user 
	printf("./Test_encode: Deconding: ./a.out -d <.bmp file> [output file]\n");
	return e_failure;
    }
    if(argc > 3)                                                                       // store the file name in output file name
    {
	strcpy(decInfo->output_file_name,argv[3]);
    }
    else
    {
	strcpy(decInfo->output_file_name,"output");
    }
    return e_success;
}
/* opening all decoding files stego_image and output file
 * Input: File info of input and output
 * Output: Proceed open decoding files
 * Return: e_success or e_failure */
Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname,"r");
    if(decInfo->fptr_stego_image == NULL)
    {
	fprintf(stderr,"Error : unable to open file %s\n",decInfo->stego_image_fname);
	return e_failure;
    }
    else
    {
	printf("INFO : Stego File opened %s\n",decInfo->stego_image_fname);
    }
    decInfo->fptr_output_file = fopen(decInfo->output_file_name,"w");
    if(decInfo->fptr_output_file == NULL)
    {
	perror("fopen");
	fprintf(stderr,"Error : unable to open file %s\n", decInfo->output_file_name);
	return e_failure;
    }
    else
    {
	printf("INFO : Output File opened %s\n",decInfo->output_file_name);
    }
    return e_success;
}
/*  decoding file stego_image to the output file
 * Input: File info of input and output
 * Output: Proceed open decoding conditions which were satisfy every decoding part of file
 * Return: e_success or e_failure */

Status do_decoding(DecodeInfo *decInfo)
{
    printf("INFO : Decoding Procedure Started\n");
    printf("INFO : Opening required files\n");
    if(open_decode_files(decInfo) == e_success)
    {
	uint data;
	fseek(decInfo->fptr_stego_image,54,SEEK_SET);
	printf("INFO : Decoding Magic String\n");

	// magic string decoding
	printf("Enter magic string\n");
	scanf("%s",decInfo->password);
	decInfo->password_size = strlen(decInfo->password);
	if(decode_magic_string(decInfo->password,decInfo->fptr_stego_image, decInfo) == e_success)
	{
	    printf("INFO : magic decoded\n");

	    //decoding ouput file extension size
	    printf("INFO : Decoding Output File Extension size\n");
	    printf("%s",decInfo->output_file_name);
	    if(decode_output_file_extn_size(decInfo->fptr_stego_image,decInfo) == e_success)
	    {
		printf("INFO : decoded extension size\n");
		if(decode_output_file_extn(decInfo->output_file_extn_size, decInfo,decInfo->fptr_stego_image) == e_success)
		{
		    printf("INFO : extension decodd\n");
		    if(decode_file_size(decInfo, decInfo->fptr_stego_image) == e_success)
		    {
			printf("INFO : file size decoded\n");
			if(decode_file_data(decInfo) == e_success)
			{
			    printf("INFO : data decoded\n");
			}
			else
			{
			    printf("decoding data failed\n");
			}
		    }
		    else
		    {
			printf("file size not decoded\n");
		    }
		}
		else
		{
		    printf("Entension decode failede\n");
		    return e_failure;
		}
	    }
	    else
	    {
		printf("Extension size not decode\n");
		return e_failure;
	    }
	}
	else
	{
	    printf("magic string not decoded\n");
	    return e_failure;
	}
    }
    else
    {
	printf("decode files are not opened\n");
	return e_failure;
    }
    return e_success;
}

/* Decodes the magic string from stego image
 * Input: Magic string and File info of input and output
 * Output: Proceed decoing if magic string if found or else stop decoding
 * Return: e_success or e_failure
 */
Status decode_magic_string( char* password, FILE *fptr,DecodeInfo *decInfo)
{
    char arr[8];
    fseek(fptr,54,SEEK_SET);
    int i;
    for(i = 0;i<decInfo->password_size;i++)
    {
	fread(arr,8,1,fptr);
	decode_lsb_to_byte(arr,&(decInfo->magic_string[i]));
    }
    decInfo -> magic_string[i] = '\0';
    // printf("%s\n", decInfo -> magic_string);
    if((strcmp(password,decInfo->magic_string)) == 0 )
    {
	printf("Password matched\n");
	return e_success;
    }
    else
    {
	printf("Password incorrect\n");
	return e_failure;
    }

}
Status decode_lsb_to_byte(char *decode_data, char *magic_string)
{
    int i,j=0;
    *magic_string = 0;

    for(i=7;i>=0;i--)
    {
	*magic_string = (decode_data[j] & 1) << i | *magic_string;
	j++;
    }
    //  printf("char = %c \n", *magic_string);
}

/* Decode file extenstion size from stego image
 * Input: File info of stego image and output file
 * Output: Decode the extenstion size from stego image and store in decode_output_extn_size
 * Return: e_success or e_failure
 */
Status decode_output_file_extn_size(FILE *fptr,DecodeInfo *decInfo) 
{
    char file_extn_size[32];
    fread(file_extn_size, 32,1, fptr);
    decode_lsb_to_size(file_extn_size,&(decInfo->output_file_extn_size));
    //printf("extn size %ld\n", decInfo->output_file_extn_size);
    return e_success;
}
Status decode_lsb_to_size(char *decode_data,long*size)
{
    int i,j=0;
    *size = 0;
    for(i=31;i>=0;i--)
    {
	*size = (decode_data[j] & 1) << i | *size;
	j++;
    }
    return e_success;
}
/* Decode file extenstion from stego image
 * Input: Extenstion Size and File info of stego image
 * Output: Decodes the file extenstion and store in decode_output_file_extn
 * Return: e_success or e_failure
 */
Status decode_output_file_extn(uint extn_size, DecodeInfo *decInfo,FILE*fptr)
{
    int i;
    //FILE *fptr = fopen(decInfo->fptr_stego_image,"r");
    char arr[8];
    for ( i = 0; i < extn_size; i++)
    {
	fread(arr, 8, 1,fptr);
	decode_lsb_to_byte(arr,&(decInfo->output_file_extn[i]));
    }
    decInfo->output_file_extn[i] ='\0';
    strcat(decInfo->output_file_name,decInfo->output_file_extn);
    //  printf("extn is: %s\n",decInfo->output_file_extn);
    //  printf("output file name :%s\n",decInfo->output_file_name);
    return e_success;
}
/* Decode file size from stego image
 * Input: ouput file and File info of stego image
 * Output: Decodes the file size and store in decode_file_size
 * Return: e_success or e_failure
 */
Status decode_file_size(DecodeInfo *decInfo,FILE *fptr)
{
    char file_size[32];
    fread(file_size, 32,1, fptr);
    decode_lsb_to_size(file_size,&(decInfo->output_file_data_size));
    // printf("file size %ld\n", decInfo->output_file_data_size);
    return e_success;
}
/* Decode file data from stego image
 * Input: Data Size and File info of stego image
 * Output: Decodes the file data and store in decode_file_data
 * Return: e_success or e_failure
 */
Status decode_file_data(DecodeInfo *decInfo)
{
    FILE *fptr = fopen(decInfo->output_file_name,"w");
    char ch,arr[8];
    for(int i = 0;i<decInfo->output_file_data_size;i++)
    {
	fread(arr,8,1,decInfo->fptr_stego_image);
	decode_lsb_to_byte(arr,&ch);
	fwrite(&ch,1,1,fptr);
    }
    fclose(fptr);
    return e_success;
}

