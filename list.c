/**
* Burak Demirci
* 141044091
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>

#define PATH_SIZE 255
/*Fonksiyon Tanimlamalari*/
void Search(const char *InpFilename, const char *target);
void DirSearch(const char *DirectoryName, const char *target );
int directoryCheck(const char *checkPath);
void CounterFile(const char *target);

int main(int argc, char const *argv[])
{
	
	if ( argc==3)
	{
		/* Fonksiyona parametreleri gonderme islemi*/
		DirSearch(argv[2],argv[1]);	
		CounterFile(argv[1]);

	}
	else
	{
		printf("Error wrong input!Try ./executeFile string directoryname\n");
	}
	return 0;
}

void DirSearch(const char *DirectoryName, const char *target )
{
	struct dirent *pathdirp;
	char pathName[PATH_SIZE];
	DIR *directoryp;
	int fileNameSize=0;
	pid_t Forkpid;

	/*Directory acilamadi hatasi */
	if ((directoryp = opendir(DirectoryName)) == NULL) 
	{
	    perror("Error opening directory"); 
	    exit(1);
	}

	/*Directory okuma islemi*/
	while((pathdirp = readdir(directoryp)) != NULL)
	{
		/*pathName DirectoryName atadim*/
		strcpy(pathName,DirectoryName);
		fileNameSize = strlen(pathdirp->d_name);
		
		if(pathdirp->d_name[fileNameSize-1]!='~')
		{
			/*Cop dosya degilse yeni yol yuklenir*/
			strcat(pathName,"/");
			strcat(pathName,pathdirp->d_name);
		}

		if(directoryCheck(pathName))
        {
			if (strcmp(pathdirp->d_name, "..") != 0 && strcmp(pathdirp->d_name, ".") != 0 )
			{
				
				if((Forkpid = fork()) == -1)
			    {
			    	/*Fork yapilamadi hatsi */
			        perror("Error doing fork\n");
			        exit(1); 
			    }
			    if (Forkpid == 0) /* Child */
			    {
				/* Child recursive olarak tekrar gelir*/
				DirSearch(pathName, target);
			        exit(0);
			    }
			    else /* Parent childi bekliyor*/
				wait(NULL);
			}
        }
        else
        {
        	Search(pathName,target);	
        }
	}
	while((closedir(directoryp) == -1) && (errno == EINTR));
}

int directoryCheck(const char *checkPath) 
{
    struct stat test;
    /* Gelen path'in directory olup olmadigini kontrol eder */
    if (stat(checkPath, &test) == -1)
        return 0;
    else
        return S_ISDIR(test.st_mode); 
}

void CounterFile(const char *target)
{
	FILE *OutFile;
	FILE *Cntr;
	int counter=0;
	int reader=0;

	OutFile = fopen("log.txt","a");
	Cntr 	= fopen("Counter.txt","r");
	if(OutFile == NULL || Cntr == NULL)
	{
		printf("Error opening file!\n");   
      	exit(1); 
	}
	while(fscanf(Cntr,"%d",&reader)!=EOF)
		counter +=reader;	
	
	/*Toplam bulunan sayisini en son log dosyasina ekleme*/
	fprintf(OutFile,"%d %s were found in total.\n",counter,target);

	fclose(Cntr);
	fclose(OutFile);

	/*Olusturulan Counter dosyasini silme */
	remove("Counter.txt");
	remove("Counter.txtt~");
}

void Search(const char *InpFilename,const char *target)
{
	FILE *input;
	FILE *OutFile;
	FILE *Cntr; /*Tekrar sayisinin icinde tutulacagi dosya*/	
	char info='y';
	int counter=1,match=1;
	int row=1,saveRow;
	int column=1,saveColumn=0;
	int seek=0;
	int numberOFind=0;
	input   = fopen(InpFilename,"r");
	OutFile = fopen("log.txt","a");
	Cntr    = fopen("Counter.txt","a");
	/*Dosya acilamadi hatasi*/
	if(input == NULL || OutFile == NULL || Cntr == NULL)
   	{
    	printf("Error opening file!\n");   
      	exit(1);             
   	}
   	/*Karakter karakter okuyup karsilastirma islemi */
   	while(fscanf(input,"%c",&info)!=EOF)
   	{
   		if(info==target[0])
   		{	/*		ilk eleman eslesti*/
   			/*Eslesen ilk karekterin konumu belirlendi */
   			saveRow=row;
   			saveColumn=column;
   			while(counter < (int)strlen(target) && fscanf(input,"%c",&info)!=EOF)
   			{
   				/*Diger eslesmeler icin yeni dongu kuruldu */
   				row++;
   				if(target[counter]==info)
   				{
   					/*Eslesme durummu*/
   					match++;
   				}	
   				if((int)info == 32 || (int)info == 9 || (int)info == 10)
   				{
   					/*Bosluk,Tab ve Newline karakteri varsa*/
   					counter--;
   					if(info==10)
   					{
   						/*Newline varsa column ve row guncellemesi*/
   						column++;
   						row=1;
   					}
   				}
   				counter++;	
   			}
   			if(counter==match && match==(int)strlen(target))
   			{
   				/*Verilen strin ile eslesme yakalandi*/
   				fprintf(OutFile," %s: [%d, %d] %s first character is found.\n",InpFilename,saveColumn,saveRow,target);
   				/*filede kac tane bulundugunu tutar*/
   				numberOFind++;
   				/*Bulunan stringin 2. elemanina geri donulup tekrar bakilmaya baslandi*/
   				fseek(input,(seek+1)*sizeof(char), SEEK_SET);
   				/*Satir ve sutunlarin eski konumuna donulmesi durumunda o konuma atanmasi*/
   				row=saveRow;
   				column=saveColumn;
   			}
   			else
   			{
   				/*Eslesmeme durumunda dosyada geri donme islemi*/
   				fseek(input,(seek+1)*sizeof(char), SEEK_SET);
   				row=saveRow;
   				column=saveColumn;
   			}
   			match=1;
   			counter=1;	
   		}
   		seek++;
   		row++;
   		if(info==10)
   		{
   			/*Newline varsa column ve row guncellemesi*/
   			column++;
   			row=1;
   		}
   	}
   	/*Bulunma sayisini dosyaya yazdirma islemi*/
   	if(numberOFind > 0)
   	{	
   		fprintf(Cntr,"%d\n",numberOFind);
   	}	
   	/*Acilan dosyayi kapatma*/
	fclose(input);
	fclose(OutFile);
	fclose(Cntr);
}
