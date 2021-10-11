#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<readline/readline.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<readline/history.h>
#include<fcntl.h>
#define MAX 200

char *komut_kutuphanesi[]={		// exec ile �al��t�r�lamayacak kodlar i�in kendim olu�turdum.
	"cd",
	"exit",
	"pwd",
	"clear"
};

int kutuphane_eleman_sayisi=sizeof(komut_kutuphanesi)/sizeof(char*);

char **command_ayir(char *command){	// char dizisi olarak verdi�im komutlar� bile�enlerine ay�ran fonksiyon.
	int size=128,i=0;				// d�n�� de�eri ��elerine ayr�lm�� komuttur. 
	char *arg;
	char **args;

	args=(char**)malloc(size*sizeof(char*));
	arg=strtok(command," \n\t\r\a");// verilen parametrelere g�re stringi ay�r�r.
	while(arg!=NULL){
	args[i]=arg;
	i++;
	arg=strtok(NULL," \n\t\a\r");
	}
	args[i]=NULL;

return args;						// Olu�turulan komut argumanlar�n� d�n.
}

char *command_oku(char **virgul_ayirilmis,int *virgul_count ){ // Terminalden inputu alan fonksiyondur. D�n�� tipi char dizisidir.
	int i=0,size=1024,j=0,dizi_idx=0;					// dizi_idx virg�l ile ayr�lm�� komutlar var ise ka� tane ayr� komut k�mesi oldu�unu tutar.
	char * buffer = (char*)malloc(size*sizeof(char));
	char **virgul_dizisi;								// Virg�l ile ayr�lm�� komutlar� gruplay�p bu matriste tutar.
	char ch;
	virgul_dizisi=(char**)malloc(sizeof(char*)*25);	
	for(i=0;i<25;i++){
		virgul_dizisi[i]=(char*)malloc(sizeof(char)*126);
	}
	i=0;
	
	printf("@asaf>");			// Terminale ad�m� yazar.
	ch=getchar();
	while(ch!='\n'){			// input bitene kadar git.
		if(ch!=';'){			// e�er ; ile ay�rmam�� isem devam et.
			buffer[i]=ch;
			i++;
		}else					// ayr�l�p yeni komut girilmi� ise ��k.
			break;
		
		ch=getchar();
	}
	if(ch==';'){				// ayr�lm�� ise yeni komutu al.
		*virgul_count+=1;		// ka� kere ayr�ld���n� tut.
		while(ch!='\n'){		// son ayr�lmada enter gelene kadar okuyacak.
			ch=getchar();
			if(ch!=';'&&ch!='\n'){	// yeni ; i�areti gelene kadar oku.
				virgul_dizisi[dizi_idx][j]=ch;
				j+=1;
			}
			else{				// yeni ; gelmi�se indis de�erlerini art�r�p yeni g�zde yeni komutu tut.
				virgul_ayirilmis[*virgul_count-1]=virgul_dizisi[dizi_idx];
				*virgul_count+=1;
				j=0;
				dizi_idx+=1;
			}
		}
}
// ilk girilen komut buffer i�erisinde. Noktal� virg�l ile ayr�lanlar ise pointerla return edilir.
return buffer;
}


int kutuphanede_varmi(char**args){ // girilen komut k�t�phane i�erisinde mi kontrol�n� yapar.
	int i;						   // olumlu ise 1 olumsuz ise 0 d�n.
	if(args[0]==NULL)
		return 0;

	for(i=0;i<kutuphane_eleman_sayisi;i++)
		if(strcmp(args[0],komut_kutuphanesi[i])==0)
			return 1;

return 0;
}

int cd_fonksiyonu(char **args){		// kendi olu�turdu�um cd fonksiyonu. �a��r�ld���nda dizinler aras� ge�i� yapar.
	chdir(args[1]);
return 1;
}

int exit_fonksiyonu(char **args){	// kendi olu�turdu�um exit fonksiyonu. Program� sonland�r�r.
return 0;
}

int pwd_fonksiyonu(char **args){	// kendi olu�turdu�um pwd g�ncel dizini verir.
	char path[256];
	if(getcwd(path,sizeof(path))!=NULL)
		fprintf(stdout,"%s\n",path);
return 1;
}

int exec_fonksiyonu(char **args){	// exec ile �al��t�rabilece�im kodlar� �al��t�r�r.
	int fork_return;
	fork_return=fork();
	if(fork_return==0){				//Child class a exec ile fonksiyon verilir.
		printf("\n");
		if(execvp(args[0],args)<0)
			printf("Command Hatasi !\n");
	}else if (fork_return==-1)
		printf("Child olusturulamadi.");
	else
		wait(NULL);  				//�ocuk olan processi bekle.
return 1;
}

int clear_fonksiyonu(void){			// kendi olu�turdu�um clear fonksiyonu clear yaz�nca �al���r.
	printf("\033[H\033[J");
return 1;							// ba�ar� ile tamamland� manas�nda 1 d�nd�r.
}

int komutu_calistir(char **args){   // girilen komut k�t�phanemdeyse yani exec ile �al��m�yorsa bunlar� gerekli fonksiyonlara y�nlendir.

	int flag;
	if(strcmp(args[0],komut_kutuphanesi[0])==0)
		flag=cd_fonksiyonu(args);

	if(strcmp(args[0],komut_kutuphanesi[1])==0)
                flag=exit_fonksiyonu(args);

	if(strcmp(args[0],komut_kutuphanesi[2])==0)
                flag=pwd_fonksiyonu(args);

	if(strcmp(args[0],komut_kutuphanesi[3])==0)
                flag=clear_fonksiyonu();

return flag;						// ba�ar�yla �al��t� ise bunun bilgisini ver.
}

int pipe_varmi(char *command,char **pipelar){// pipe var ise 1 ve pipelar matrisini dondurecek yok ise 0
	int i=0,flag=1;
	while(flag){
		pipelar[i]=strsep(&command ,"|");
		if(pipelar[i]==NULL)
			flag=0;
	i++;
	}
	if(pipelar[1]==NULL)
		return 0;		// Eger | yok ise ikinci k�s�m null olacak.
	else{
		//return 1;	// �kinci k�s�m dolu ise pipe vard�r.
		//printf("pipe ilk : %s\npipe_iki : %s\n",pipelar[0],pipelar[1]);
		return 1;
	}
}

int pipe_komutu_calistir(char** pipe_ilk,char** pipe_iki){ // Girilen pipe komutlar�n� �al��t�r�r. D�zg�nce �al���rsa 1 d�nd�r�r.
	int pipe_uclari[2];
	pid_t fork_ret1,fork_ret2;				// fork sonucu olu�an de�erleri tutar.
//	pid_t ilk_child,iki_child;				// childlar� tutar.
	if(pipe(pipe_uclari)<0){				// pipe olu�turma fonksiyonu.
		printf("Pipe olusturulamadi.\n");
		return 0;
}

	fork_ret1=fork();

	if(fork_ret1==0){// Child process ise.
		//printf("1. Child olustu.\n");
		//ilk_child=getpid();
		close(pipe_uclari[0]);				// pipe �n ilk ucunu kapat�r.
		dup2(pipe_uclari[1],fileno(stdout));// ikinci uca veriyi ��kar.
		close(pipe_uclari[1]);				// ikinci ucu kapat�r.
		if(execvp(pipe_ilk[0],pipe_ilk)<0){ // ilk u�tan ��k�lacak de�eri �al��t�r�r.
			printf("Komut yurutulemedi.\n1.Child sonlan�yor.\n");
			exit(0);
		}
	//printf("1.Child terminate edildi.\n");
	exit(0);								// ilk child � sonland�r.
	} else{// Parent process ise.
		fork_ret2=fork();
		if(fork_ret2==0){
			//printf("2. Child olustu.\n");
			//iki_child=getpid();
			close(pipe_uclari[1]);			// ikinci ucu kapat
			dup2(pipe_uclari[0],fileno(stdin));// ilk u�tan veriyi oku.
			close(pipe_uclari[0]);			// ilk ucu kapat.
			if(execvp(pipe_iki[0],pipe_iki)<0){//kendi fonksiyonunu �al��t�r.
				printf("Komut yurutulemedi.\n2.Child sonlan�yor.");
				exit(0);
			}else
				exit(0);					// ikinci child sonland�r.
		//printf("2. Child terminate edildi.\n");
		exit(0);
		}else{// Parent ise iki child � da beklesin.
			//printf("Parent beklemeye basladi.\n");
			close(pipe_uclari[0]);			// her iki ucu da kapat.
			close(pipe_uclari[1]);
			wait(NULL);						// �ocuklar� bekle.
			//printf("Ilk child geldi.\n");
			wait(NULL);
			//printf("Parent beklemeyi bitirdi.\n");
		}
	}
return 1;
}

int main(void){
	char*command;							// girilen inputu tutar.
	char**command_w_args;					// girilen inputun argumanlar�na ayr�lm�� halini tutar.	
	int flag=1,i=0,size=1024,count=0;
	char *pipe_tutucu[2];					// pipe u�lar�n� tutar.
	char **pipe_ilk;			
	char **pipe_iki;
	char **virgul_ayirilmis;				// virg�l ile ayr�lm�� komutlar girilirse bunlar� tutar.
	int virgul_count=0;
	int pipe_durumu;
	virgul_ayirilmis=(char**)malloc(sizeof(char*)*MAX);
	
	while(flag){
		//printf("@asaf");
		command=command_oku(virgul_ayirilmis,&virgul_count);  // girilen inputu al.
		//printf("first komut : %s",command);
		if(virgul_count>=1){								// virg�l var ise.
			for(i=0;i<=virgul_count;i++){					// s�ra ile komutlar� �al��t�r.
				//printf("Dongudeyim %d\n",i);
				pipe_durumu=pipe_varmi(command,pipe_tutucu);// pipe var m� bilgisini tutar.
                		if(pipe_durumu){					// pipe var ise
                        		pipe_ilk=command_ayir(pipe_tutucu[0]);  // ilk k�sm�n argumanlar�n� al
                        		pipe_iki=command_ayir(pipe_tutucu[1]);  // ikinci k�sm�n argumanlar�n� al.
                        		flag=pipe_komutu_calistir(pipe_ilk,pipe_iki); // �al��t�r.
                		}
                		else{									  // pipe yok ise
                        	command_w_args=command_ayir(command); // argumanlara ay�r.
							if(command_w_args[0]!=NULL){		  //Komut girilmi�se
                                	if(kutuphanede_varmi(command_w_args))//girilen komut k�t�phanede ise.
                                        flag=komutu_calistir(command_w_args); // y�nlendir.
                                	else
										flag=exec_fonksiyonu(command_w_args); // de�il ise exec ile �al��t�r.
							}
						}
			//printf("%s\n",virgul_ayirilmis[i]);
				command=virgul_ayirilmis[i];				// Virg�l ile ayr�lm�� ise s�radaki �al��t�r�lacak komutu ver.
			//printf("yeni command verdim : %s\n",command);
			}
		}else{
			pipe_durumu=pipe_varmi(command,pipe_tutucu);	// Virg�l yok ise
                	if(pipe_durumu){						// pipe var ise
                        	pipe_ilk=command_ayir(pipe_tutucu[0]);	// ilk k�sm� argumanlar�na ay�r.
                        	pipe_iki=command_ayir(pipe_tutucu[1]);  // ikinci k�sm� argumanlar�n ay�r.
                        	flag=pipe_komutu_calistir(pipe_ilk,pipe_iki);// �al��t�r.
                	}
                	else{
                        command_w_args=command_ayir(command);		// pipe yok ise 
                        if(command_w_args[0]!=NULL){				//Komut girilmi�se
                                if(kutuphanede_varmi(command_w_args))//girilen komut k�t�phanede ise
                                    flag=komutu_calistir(command_w_args);
                                else								// de�il ise exec ile �al��t�r.
                                    flag=exec_fonksiyonu(command_w_args);
						}
					}
			}
		virgul_count=0;		// virg�lle ayr�lan komut say�s�n� s�f�rla.

	
	}
return 0;
}
