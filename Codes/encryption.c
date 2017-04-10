#include<stdio.h>
#include<string.h>
#include<time.h>
#include<stdlib.h>

#define KEYS 11

char* key_matrix[KEYS];
int sbox[4][16] = {{5,14,6,13,7,4,2,10,8,12,0,9,1,11,15,3},{12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11},{4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1},{13,9,15,12,11,5,7,6,3,8,14,2,0,1,4,10}};
int permu[64] = {63,55,47,39,31,23,15,7,62,54,46,38,30,22,14,6,61,53,45,37,29,21,13,5,60,52,44,36,28,20,12,4,59,51,43,35,27,19,11,3,58,50,42,34,26,18,10,2,57,49,41,33,25,17,9,1,56,48,40,32,24,16,8,0};

char* printbincharpad(char c)
{
	char* y = malloc(10*sizeof(char));
    int f = c;
	for(int h=0;h<8;h++)  
	{  
		y[7-h]=f%2+'0';  
		f=f/2;  
	}y[8]='\0';
    return y;
}

void gen_ascii_file()
{
		char* num = malloc(50*sizeof(char));
        FILE *fp1, *fp2;
        char ch;
        fp1 = fopen("alice.txt", "r");
        if (!fp1) {
                printf("Unable to open the input file!!\n");
                exit(0);
        }
        fp2 = fopen("ascii_alice.txt", "w");
        if (!fp2) {
                printf("Unable to open the output file!!\n");
                exit(0);
        }
        int count=0;
        while (!feof(fp1)) 
        {
                fread(&ch, sizeof(char), 1, fp1);
                //printf("%c\n",ch );
                strcpy(num,printbincharpad(ch));
                //fwrite(&num, sizeof(int), 1, fp2);
                fprintf(fp2, "%s",num );
                count++;
        }
        while(count%16!=0)
        {
        	strcpy(num,printbincharpad(' '));
            //fwrite(&num, sizeof(int), 1, fp2);
            fprintf(fp2, "%s",num );
            count++;	
        }
        fclose(fp1);
        fclose(fp2);
}

void gen_subkeys()
{	
	srand(time(NULL));
	FILE* out;
	out = fopen("keys.txt","w");
	for(int i =0;i<11;i++)
	{
		key_matrix[i] = malloc(65*sizeof(char));
		for(int j=0;j<64;j++)
		{
			key_matrix[i][j]=(rand()%2)+'0';
			fprintf(out,"%c",key_matrix[i][j]);
		}
		key_matrix[i][64]='\0';
		fprintf(out,"%s","\n");
	}
	fclose(out);
}

char* XOR(char* a,char* b)
{
  char* ans = malloc(strlen(a)*sizeof(char));
  int ab = strlen(a);

  for(int i=0;i<ab;i++)
  {
	if(a[i]==b[i]){
	  ans[i]='0';
	}
	else{
	  ans[i]='1';
	}
  }
  ans[ab]='\0';
  //printf("XOR- %s\n",ans);
  return ans;
}

char* Permute(char* input)
{
	char* output = malloc(64*sizeof(char));
	for(int i=0;i<64;i++)
	{
		output[permu[i]]=input[i];
	}
	//printf("\nFEISTEL - %s\n",output);
	return output;
}

char* Substitute(char* input)
{
	char* output = malloc(65*sizeof(char));
	strcpy(output,"");
	for(int i=0;i<4;i++)
	{
		int count = 0;
		for(int j=0;j<4;j++)
		{
			int a = input[16*i+4*j+0]-'0';
			int b = input[16*i+4*j+1]-'0';
			int c = input[16*i+4*j+2]-'0';
			int d = input[16*i+4*j+3]-'0';
			int e = 8*a+4*b+2*c+d;
			int f = sbox[count][e];
			char* y = malloc(5*sizeof(char));
			for(int h=0;h<4;h++)  
			{  
				y[3-h]=f%2+'0';  
				f=f/2;  
			}y[4]='\0';
			//printf("%d %d %s\n",e,f,y);
			strcat(output,y);
			count++;
		}
	}
	return output;
}

char* Fiestel(char* R0,int r)
{
	char* subkey = malloc(64*sizeof(char));
	char* input = malloc(64*sizeof(char));
	strcpy(subkey,key_matrix[r]);
	strcpy(input,XOR(R0,subkey));
	char* substituted = malloc(64*sizeof(char));
	strcpy(substituted,Substitute(input));
	//printf("SUB- %s\n",substituted);
	return Permute(substituted);
}  

char* Round(char* mssg_block,int r)
{
	char* L0;
	L0 = malloc(64*sizeof(char));
	char* R0;
	R0 = malloc(64*sizeof(char));
	char* L1;
	L1 = malloc(64*sizeof(char));
	char* R1;
	R1 = malloc(64*sizeof(char));
	for(int k=0;k<64;k++)
	{
		L0[k] = mssg_block[k];
		R0[k] = mssg_block[k+64];
		L1[k] = R0[k];
	}
	strcpy(R1,XOR(Fiestel(R0,r),L0));
	
	return strcat(L1,R1);
}

int main(int argc,char* argv)
{
	//printf("%s\n",Permute("0111110010010011011110000101100101100110101100100101000000111011"));
	int i = 0;
	int j = 0;
	clock_t start_t, end_t, total_t;
	FILE* inp;
	char* plaintext;
	gen_ascii_file();
	long input_file_size;
	inp = fopen("ascii_alice.txt","r");
	fseek(inp, 0, SEEK_END);
	input_file_size = ftell(inp);
	rewind(inp);
	plaintext = malloc(input_file_size * (sizeof(char)));
	fread(plaintext, sizeof(char), input_file_size, inp);
	printf("%ld\n",input_file_size);
	fclose(inp);
	long plaintext_blocks = strlen(plaintext)/128;
	gen_subkeys();
	printf("%ld\n",plaintext_blocks);
	//strcpy(key_matrix[0],"0001100010010101100110111101100100000101110000101110100000010001");
	inp = fopen("enc_alice.txt","w");

	char* mssg_block1;
	mssg_block1 = malloc(128*sizeof(char));
	for(int k=0;k<128;k++)
		mssg_block1[k]='0';
	
	for(i=0;i<33;i++)
	{
		strcpy(mssg_block1,Round(mssg_block1,i%11));
		//printf("%s\n",mssg_block );
	}
	start_t = clock();
	for(j=0;j<plaintext_blocks;j++)
	{
		char* mssg_block;
		mssg_block = malloc(128*sizeof(char));
		for(int k=0;k<128;k++)
			mssg_block[k]=plaintext[128*j+k];
		
		for(i=0;i<33;i++)
		{
			strcpy(mssg_block,Round(mssg_block,i%11));
			//printf("%s\n",mssg_block );
		}
		fprintf(inp,"%s",mssg_block );
	}
	end_t = clock();
   	 double total_t1 = ((double)end_t - start_t) / CLOCKS_PER_SEC;
   	printf("Total time taken by CPU: %ld\n", start_t  );
   	printf("Total time taken by CPU: %ld\n", end_t  );
   	printf("Total time taken by CPU: %f\n", total_t1  );
	fclose(inp);
	return 0;
}
