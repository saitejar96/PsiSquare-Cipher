#include<stdio.h>
#include<string.h>
#include<time.h>
#include<stdlib.h>

#define KEYS 32

char* key_matrix[KEYS];
int sbox[4][16] = {{5,14,6,13,7,4,2,10,8,12,0,9,1,11,15,3},{12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11},{4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1},{13,9,15,12,11,5,7,6,3,8,14,2,0,1,4,10}};
int permu[64] = {63,55,47,39,31,23,15,7,62,54,46,38,30,22,14,6,61,53,45,37,29,21,13,5,60,52,44,36,28,20,12,4,59,51,43,35,27,19,11,3,58,50,42,34,26,18,10,2,57,49,41,33,25,17,9,1,56,48,40,32,24,16,8,0};

void gen_text_file()
{
		char* num = malloc(50*sizeof(char));
        FILE *fp1, *fp2;
        char* ch = malloc(50*sizeof(char));
        fp1 = fopen("dec_alice.txt", "r");
        if (!fp1) {
                printf("Unable to open the input file!!\n");
                exit(0);
        }
        fp2 = fopen("new_alice.txt", "w");
        if (!fp2) {
                printf("Unable to open the output file!!\n");
                exit(0);
        }
        while (!feof(fp1)) 
        {
                fread(ch, sizeof(char), 8, fp1);
                ch[8]='\0';
                //printf("%c\n",ch );
                char result;
                int x = 128*(ch[0]-'0')+64*(ch[1]-'0')+32*(ch[2]-'0')+16*(ch[3]-'0')+8*(ch[4]-'0')+4*(ch[5]-'0')+2*(ch[6]-'0')+1*(ch[7]-'0');
                result = x;
                printf("%s %c\n",ch,result);
                //fwrite(&num, sizeof(int), 1, fp2);
                fprintf(fp2, "%c",result);
        }
        fclose(fp1);
        fclose(fp2);
}

void gen_subkeys()
{	
	srand(time(NULL));
	FILE* out;
	out = fopen("keys.txt","r");
	for(int i =10;i>=0;i--)
	{
		key_matrix[i] = malloc(65*sizeof(char));
		for(int j=0;j<64;j++)
		{
			key_matrix[i][j]=fgetc(out);
		}
		key_matrix[i][64]='\0';
		fgetc(out);
		printf("%s\n",key_matrix[i]);
	}
	fclose(out);
}

char* XOR(char* a,char* b)
{
  char* ans = malloc(65*sizeof(char));
  for(int i=0;i<64;i++)
  {
	if(a[i]==b[i]){
	  ans[i]='0';
	}
	else{
	  ans[i]='1';
	}
  }ans[64]='\0';
  return ans;
}

char* Permute(char* input)
{
	char* output = malloc(65*sizeof(char));
	for(int i=0;i<64;i++)
	{
		output[permu[i]]=input[i];
	}
	output[64]='\0';
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
	//printf("!!!!%s\n",input);
	char* substituted = malloc(64*sizeof(char));
	strcpy(substituted,Substitute(input));
	return Permute(substituted);
}  

char* Round(char* mssg_block,int r)
{	//return mssg_block;
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
		R1[k] = L0[k];
	}
	//printf("*****%s\n",R1);
	strcpy(L1,XOR(Fiestel(L0,r),R0));
	//printf("!!!!!!!!%s!!!!!!!\n~~~~~~~~~%s~~~~~~~~\n",R1,L1);
	return strcat(L1,R1);
}

int main(int argc,char* argv)
{
	int i = 0;
	int j = 0;
	FILE* inp;
	char* plaintext;
	long input_file_size;
	inp = fopen("enc_alice.txt","r");
	fseek(inp, 0, SEEK_END);
	input_file_size = ftell(inp);
	rewind(inp);
	plaintext = malloc(input_file_size * (sizeof(char)));
	fread(plaintext, sizeof(char), input_file_size, inp);
	fclose(inp);
	long plaintext_blocks = strlen(plaintext)/128;
	gen_subkeys();
	//strcpy(key_matrix[0],"0001100010010101100110111101100100000101110000101110100000010001");
	printf("subkeys found %ld\n",plaintext_blocks);
	inp = fopen("dec_alice.txt","w");
	//fprintf(inp,"%s",plaintext );
	for(j=0;j<plaintext_blocks;j++)
	{
		char* mssg_block;
		mssg_block = malloc(128*sizeof(char));
		for(int k=0;k<128;k++)
			mssg_block[k]=plaintext[128*j+k];
		
		for(i=0;i<33;i++)
		{
			strcpy(mssg_block,Round(mssg_block,i%11));
			//printf("%s@@@@@@@@\n",mssg_block );
		}
		fprintf(inp,"%s",mssg_block );
	}
	fclose(inp);
	gen_text_file();
	return 0;
}
