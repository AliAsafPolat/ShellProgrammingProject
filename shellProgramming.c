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

char *komut_kutuphanesi[]={		// exec ile çalýþtýrýlamayacak kodlar için kendim oluþturdum.
	"cd",
	"exit",
	"pwd",
	"clear"
};

int kutuphane_eleman_sayisi=sizeof(komut_kutuphanesi)/sizeof(char*);

char **command_ayir(char *command){	// char dizisi olarak verdiðim komutlarý bileþenlerine ayýran fonksiyon.
	int size=128,i=0;				// dönüþ deðeri öðelerine ayrýlmýþ komuttur. 
	char *arg;
	char **args;

	args=(char**)malloc(size*sizeof(char*));
	arg=strtok(command," \n\t\r\a");// verilen parametrelere göre stringi ayýrýr.
	while(arg!=NULL){
	args[i]=arg;
	i++;
	arg=strtok(NULL," \n\t\a\r");
	}
	args[i]=NULL;

return args;						// Oluþturulan komut argumanlarýný dön.
}

char *command_oku(char **virgul_ayirilmis,int *virgul_count ){ // Terminalden inputu alan fonksiyondur. Dönüþ tipi char dizisidir.
	int i=0,size=1024,j=0,dizi_idx=0;					// dizi_idx virgül ile ayrýlmýþ komutlar var ise kaç tane ayrý komut kümesi olduðunu tutar.
	char * buffer = (char*)malloc(size*sizeof(char));
	char **virgul_dizisi;								// Virgül ile ayrýlmýþ komutlarý gruplayýp bu matriste tutar.
	char ch;
	virgul_dizisi=(char**)malloc(sizeof(char*)*25);	
	for(i=0;i<25;i++){
		virgul_dizisi[i]=(char*)malloc(sizeof(char)*126);
	}
	i=0;
	
	printf("@asaf>");			// Terminale adýmý yazar.
	ch=getchar();
	while(ch!='\n'){			// input bitene kadar git.
		if(ch!=';'){			// eðer ; ile ayýrmamýþ isem devam et.
			buffer[i]=ch;
			i++;
		}else					// ayrýlýp yeni komut girilmiþ ise çýk.
			break;
		
		ch=getchar();
	}
	if(ch==';'){				// ayrýlmýþ ise yeni komutu al.
		*virgul_count+=1;		// kaç kere ayrýldýðýný tut.
		while(ch!='\n'){		// son ayrýlmada enter gelene kadar okuyacak.
			ch=getchar();
			if(ch!=';'&&ch!='\n'){	// yeni ; iþareti gelene kadar oku.
				virgul_dizisi[dizi_idx][j]=ch;
				j+=1;
			}
			else{				// yeni ; gelmiþse indis deðerlerini artýrýp yeni gözde yeni komutu tut.
				virgul_ayirilmis[*virgul_count-1]=virgul_dizisi[dizi_idx];
				*virgul_count+=1;
				j=0;
				dizi_idx+=1;
			}
		}
}
// ilk girilen komut buffer içerisinde. Noktalý virgül ile ayrýlanlar ise pointerla return edilir.
return buffer;
}


int kutuphanede_varmi(char**args){ // girilen komut kütüphane içerisinde mi kontrolünü yapar.
	int i;						   // olumlu ise 1 olumsuz ise 0 dön.
	if(args[0]==NULL)
		return 0;

	for(i=0;i<kutuphane_eleman_sayisi;i++)
		if(strcmp(args[0],komut_kutuphanesi[i])==0)
			return 1;

return 0;
}

int cd_fonksiyonu(char **args){		// kendi oluþturduðum cd fonksiyonu. Çaðýrýldýðýnda dizinler arasý geçiþ yapar.
	chdir(args[1]);
return 1;
}

int exit_fonksiyonu(char **args){	// kendi oluþturduðum exit fonksiyonu. Programý sonlandýrýr.
return 0;
}

int pwd_fonksiyonu(char **args){	// kendi oluþturduðum pwd güncel dizini verir.
	char path[256];
	if(getcwd(path,sizeof(path))!=NULL)
		fprintf(stdout,"%s\n",path);
return 1;
}

int exec_fonksiyonu(char **args){	// exec ile çalýþtýrabileceðim kodlarý çalýþtýrýr.
	int fork_return;
	fork_return=fork();
	if(fork_return==0){				//Child class a exec ile fonksiyon verilir.
		printf("\n");
		if(execvp(args[0],args)<0)
			printf("Command Hatasi !\n");
	}else if (fork_return==-1)
		printf("Child olusturulamadi.");
	else
		wait(NULL);  				//Çocuk olan processi bekle.
return 1;
}

int clear_fonksiyonu(void){			// kendi oluþturduðum clear fonksiyonu clear yazýnca çalýþýr.
	printf("\033[H\033[J");
return 1;							// baþarý ile tamamlandý manasýnda 1 döndür.
}

int komutu_calistir(char **args){   // girilen komut kütüphanemdeyse yani exec ile çalýþmýyorsa bunlarý gerekli fonksiyonlara yönlendir.

	int flag;
	if(strcmp(args[0],komut_kutuphanesi[0])==0)
		flag=cd_fonksiyonu(args);

	if(strcmp(args[0],komut_kutuphanesi[1])==0)
                flag=exit_fonksiyonu(args);

	if(strcmp(args[0],komut_kutuphanesi[2])==0)
                flag=pwd_fonksiyonu(args);

	if(strcmp(args[0],komut_kutuphanesi[3])==0)
                flag=clear_fonksiyonu();

return flag;						// baþarýyla çalýþtý ise bunun bilgisini ver.
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
		return 0;		// Eger | yok ise ikinci kýsým null olacak.
	else{
		//return 1;	// Ýkinci kýsým dolu ise pipe vardýr.
		//printf("pipe ilk : %s\npipe_iki : %s\n",pipelar[0],pipelar[1]);
		return 1;
	}
}

int pipe_komutu_calistir(char** pipe_ilk,char** pipe_iki){ // Girilen pipe komutlarýný çalýþtýrýr. Düzgünce çalýþýrsa 1 döndürür.
	int pipe_uclari[2];
	pid_t fork_ret1,fork_ret2;				// fork sonucu oluþan deðerleri tutar.
//	pid_t ilk_child,iki_child;				// childlarý tutar.
	if(pipe(pipe_uclari)<0){				// pipe oluþturma fonksiyonu.
		printf("Pipe olusturulamadi.\n");
		return 0;
}

	fork_ret1=fork();

	if(fork_ret1==0){// Child process ise.
		//printf("1. Child olustu.\n");
		//ilk_child=getpid();
		close(pipe_uclari[0]);				// pipe ýn ilk ucunu kapatýr.
		dup2(pipe_uclari[1],fileno(stdout));// ikinci uca veriyi çýkar.
		close(pipe_uclari[1]);				// ikinci ucu kapatýr.
		if(execvp(pipe_ilk[0],pipe_ilk)<0){ // ilk uçtan çýkýlacak deðeri çalýþtýrýr.
			printf("Komut yurutulemedi.\n1.Child sonlanýyor.\n");
			exit(0);
		}
	//printf("1.Child terminate edildi.\n");
	exit(0);								// ilk child ý sonlandýr.
	} else{// Parent process ise.
		fork_ret2=fork();
		if(fork_ret2==0){
			//printf("2. Child olustu.\n");
			//iki_child=getpid();
			close(pipe_uclari[1]);			// ikinci ucu kapat
			dup2(pipe_uclari[0],fileno(stdin));// ilk uçtan veriyi oku.
			close(pipe_uclari[0]);			// ilk ucu kapat.
			if(execvp(pipe_iki[0],pipe_iki)<0){//kendi fonksiyonunu çalýþtýr.
				printf("Komut yurutulemedi.\n2.Child sonlanýyor.");
				exit(0);
			}else
				exit(0);					// ikinci child sonlandýr.
		//printf("2. Child terminate edildi.\n");
		exit(0);
		}else{// Parent ise iki child ý da beklesin.
			//printf("Parent beklemeye basladi.\n");
			close(pipe_uclari[0]);			// her iki ucu da kapat.
			close(pipe_uclari[1]);
			wait(NULL);						// çocuklarý bekle.
			//printf("Ilk child geldi.\n");
			wait(NULL);
			//printf("Parent beklemeyi bitirdi.\n");
		}
	}
return 1;
}

int main(void){
	char*command;							// girilen inputu tutar.
	char**command_w_args;					// girilen inputun argumanlarýna ayrýlmýþ halini tutar.	
	int flag=1,i=0,size=1024,count=0;
	char *pipe_tutucu[2];					// pipe uçlarýný tutar.
	char **pipe_ilk;			
	char **pipe_iki;
	char **virgul_ayirilmis;				// virgül ile ayrýlmýþ komutlar girilirse bunlarý tutar.
	int virgul_count=0;
	int pipe_durumu;
	virgul_ayirilmis=(char**)malloc(sizeof(char*)*MAX);
	
	while(flag){
		//printf("@asaf");
		command=command_oku(virgul_ayirilmis,&virgul_count);  // girilen inputu al.
		//printf("first komut : %s",command);
		if(virgul_count>=1){								// virgül var ise.
			for(i=0;i<=virgul_count;i++){					// sýra ile komutlarý çalýþtýr.
				//printf("Dongudeyim %d\n",i);
				pipe_durumu=pipe_varmi(command,pipe_tutucu);// pipe var mý bilgisini tutar.
                		if(pipe_durumu){					// pipe var ise
                        		pipe_ilk=command_ayir(pipe_tutucu[0]);  // ilk kýsmýn argumanlarýný al
                        		pipe_iki=command_ayir(pipe_tutucu[1]);  // ikinci kýsmýn argumanlarýný al.
                        		flag=pipe_komutu_calistir(pipe_ilk,pipe_iki); // çalýþtýr.
                		}
                		else{									  // pipe yok ise
                        	command_w_args=command_ayir(command); // argumanlara ayýr.
							if(command_w_args[0]!=NULL){		  //Komut girilmiþse
                                	if(kutuphanede_varmi(command_w_args))//girilen komut kütüphanede ise.
                                        flag=komutu_calistir(command_w_args); // yönlendir.
                                	else
										flag=exec_fonksiyonu(command_w_args); // deðil ise exec ile çalýþtýr.
							}
						}
			//printf("%s\n",virgul_ayirilmis[i]);
				command=virgul_ayirilmis[i];				// Virgül ile ayrýlmýþ ise sýradaki çalýþtýrýlacak komutu ver.
			//printf("yeni command verdim : %s\n",command);
			}
		}else{
			pipe_durumu=pipe_varmi(command,pipe_tutucu);	// Virgül yok ise
                	if(pipe_durumu){						// pipe var ise
                        	pipe_ilk=command_ayir(pipe_tutucu[0]);	// ilk kýsmý argumanlarýna ayýr.
                        	pipe_iki=command_ayir(pipe_tutucu[1]);  // ikinci kýsmý argumanlarýn ayýr.
                        	flag=pipe_komutu_calistir(pipe_ilk,pipe_iki);// çalýþtýr.
                	}
                	else{
                        command_w_args=command_ayir(command);		// pipe yok ise 
                        if(command_w_args[0]!=NULL){				//Komut girilmiþse
                                if(kutuphanede_varmi(command_w_args))//girilen komut kütüphanede ise
                                    flag=komutu_calistir(command_w_args);
                                else								// deðil ise exec ile çalýþtýr.
                                    flag=exec_fonksiyonu(command_w_args);
						}
					}
			}
		virgul_count=0;		// virgülle ayrýlan komut sayýsýný sýfýrla.

	
	}
return 0;
}
