#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NAME_LEN 100

//Hashtable i�in struct yap�s�
typedef struct{
    char name[MAX_NAME_LEN];
    char type[10];
    int flag; // 0: bo�, 1: dolu
}Symbol;

/*
@brief Say�n�n asal olup olmad���n� kontrol eder
@param n Asall��� kontrol edilecek say�
@return 1 ile d�nerse say� asald�r. 0 ile d�nerse say� asal de�ildir.
*/
int isPrime(int n){
    int i;
    if(n<2) return 0;
    for(i=2;i*i<=n;i++){
        if(n%i==0) return 0;
    }
    return 1;
}

/*
@brief Verilen say�dan sonraki asal say�y� bulur.
@param n kendisinden sonraki asal say� bulunmas� istenen say�
@return Sonraki asal say� d�nd�r�l�r.
*/
int nextPrime(int n){
    while(!isPrime(n))n++;
    return n;
}

/*
@brief Tabloya yerle�tirilecek de�i�kenlerin isimlerini hash fonksiyonuna verebilmek i�in say� kar��l�klar� Horner methodu ile hesaplan�r.
@param str horner methodu ile keyi hesaplanacak de�i�ken
@return Say�sal de�eri hesaplanm�� de�i�ken
*/
int hornerKey(char *str){
    int key=0,i;
    for (i=0;str[i]!='\0';i++){
        key=key*31+str[i]; // Horner kural�
    }
    return key;
}

/*
@brief ilk hash fonksiyonu
@param key horner methoduyla hesaplanm�� de�er
@param m tablo uzunlu�u
*/
int h1(int key,int m){
    return key % m;
}

/*
@brief ikinci hash fonksiyonu
@param key horner methoduyla hesaplanm�� de�er
@param m tablo uzunlu�u
*/
int h2(int key,int m) {
    return 1+(key%(m-3));
}

/*
@brief iki hash fonksiyonunu kullanarak de�i�kenin tabloda yerle�ecek yerini bulur.
@param key horner methoduyla hesaplanm�� de�er
@param i tablodaki ilgili yer doluysa double hash ile sonraki bo� yeri bulmaya yarar.
@param m tablo uzunlu�u
*/
int h(int key,int i,int m){
    return (h1(key, m)+i*h2(key, m))%m;
}

/*
@brief Dosyadan string de�erlerini okur ve token olarak her bir stringi ay�r�r.
@param filename a��lacak dosyan�n ismi
@param token_count a��lacak dosyada token yani string say�s�n� saklar.
@return tokens par�alanm�� stringleri tutan string dizisinin adresini d�nd�r�r.
*/
char **readFile(char *filename,int *token_count){
    FILE *file=fopen(filename, "r");
    if(!file){
        printf("Hata: Dosya a��lamad�.\n");
        return NULL;
    }

    char line[256];
    char **tokens=malloc(100*sizeof(char *)); // Token dizisi i�in bellek ay�r
    *token_count=0;

    while(fgets(line,sizeof(line),file)){    // Sat�r sat�r dosyay� okur ve stringleri verilen i�aretleri kald�rarak ve ona g�re ay�rarak token dizisine atar.
        char *token = strtok(line, " <>\t=%+-/[]{},()\n");  // Noktal� virg�l� kald�r�lacaklar dizisine eklemedim ona g�re i�lem yapaca��m.
        while(token){
            tokens[*token_count]=malloc(strlen(token)+1);
            strcpy(tokens[*token_count], token);
            (*token_count)++;
            token=strtok(NULL, "  <>\t=%+-/[]{},()\n");
        }
    }

    fclose(file);
    return tokens;
}

/*
@brief Token dizisini yani par�alanm�� stringleri ekrana yazd�r�r.
@param tokens token dizisi 
@param token_count toplam token say�s�
*/
void printTokens(char **tokens,int token_count){
    int i;
    for (i=0;i<token_count;i++) {
        printf("Token %d: %s\n",i+1,tokens[i]);
    }
}

/*
@brief  Tokenleri gezerek, tan�mlanmaya �al��an de�i�ken say�s�n� bularak onun boyutu kadar uzunlukta sembol tablosunu olu�turur.
@param tokens token dizisi
@param token_count token say�s�
@param m tablonun uzunlu�u
@return sembol tablosu yani hashtable
*/
Symbol *createTable(char **tokens,int token_count,int *m){
    int i=0,variable_count=0,start_index=0,finish_index=0;
    Symbol *symbol_table;

    while(i<token_count){
        if (strcmp(tokens[i],"int")== 0 || strcmp(tokens[i],"float")==0 || strcmp(tokens[i], "char")==0){ //Bu 3 stringten birisi varsa o sat�rda deklare i�lemi yap�lcakt�r.
            start_index = i; //Deklare yap�lacak token indisini saklad�k.
        }
        if(strchr(tokens[i],';')!=NULL){ // ';' i�eren token varsa
            finish_index=i; //Sat�r sonuna geldik demektir. Start indisindeki tipte de�i�kenler deklare edildi�ini anl�yoruz.
            if(start_index>0){ //Start indisi 0'dan b�y�kse yani de�i�ken tan�mlan�yorsa 
                variable_count+=(finish_index-start_index);// �lgili sat�rda ka� de�i�ken tan�mland���n� bu �ekilde bulabiliriz.
                start_index=0; //Sonras�nda deklare i�lemi yoksa variable_count say�s�na etki etmesin ve bu if'e girmesin diye 0 yap�yoruz. 
				//Deklare i�lemi yap�l�yorsa bu tekrardan 0'dan farkl� bir say� olur zaten. 
            }
        }
        i++;
    }

    *m=nextPrime(variable_count*2);  //�ki kat�ndan sonraki asal say�s� kadar tablo boyutu olu�turuyoruz.
    symbol_table=(Symbol *)calloc(*m, sizeof(Symbol));
    if (!symbol_table){
        printf("Hata: Bellek tahsis edilemedi.\n");
        return NULL;
    }
    return symbol_table;
}

/*
@brief Tablonun ilgili g�z� bo�sa oraya gerekli de�i�keni ekler.
@param table sembol yani hashtable
@param m tablo uzunlu�u
@param name eklenmesi istenilen de�i�kenin ismi
@param type eklenmesi istenilen de�i�kenin tipi
*/
void insert(Symbol *table,int m,char *name,char *type){
    int i;
    int key=hornerKey(name);
    for (i=0;i<m;i++){
        int idx=h(key,i,m);
        if(table[idx].flag==0){
            strcpy(table[idx].name,name);
            strcpy(table[idx].type,type);
            table[idx].flag=1;
            return;
        }
    }
    printf("Hata: Hash tablosu dolu, '%s' eklenemedi.\n",name);
}

/*
@brief tabloda parametre olarak verilen de�i�kenin �nceden tabloya eklenip eklenmedi�ini kontrol eder.
@param table sembol tablosu
@param m tablo uzunlu�u
@param name tabloda var m� diye kontrol edilmesi istenen de�i�ken
@return E�er tabloda o de�erin ayn�s� �nceden eklenmi�se 1 eklenmemi�se 0 de�erini d�nd�r�r.
*/
int lookup(Symbol *table,int m,char *name){
	int i=0;
    int key=hornerKey(name);
    while(i<m){  //Tabloyu gezer.
        int idx=h(key, i, m);
        if(strcmp(table[idx].name, name)==0){  //Ayn� isimde de�i�ken var m� diye k�yaslar.
        	return 1;
		}      
        i++;
    }
    return 0;
}

/*
@brief Tabloya ekleme ve hatalar� kontrol i�leri burada olur.
@param tokens token dizisi
@param token_count token say�s�
@param symbol_table sembol tablosu
@param m tablo uzunlu�u
*/
void insertTable(char **tokens,int token_count,Symbol *symbol_table,int m){
    int i=0,j,start_index = 0,finish_index = 0,finish_declare=0;
    char *tmp;

    while(i<token_count){  //Variable say�s� hesaplamada yapt���m�z i�leme benzer bir �ekilde i�lem yapaca��z.
        if(strcmp(tokens[i],"int")==0 || strcmp(tokens[i],"float")==0 || strcmp(tokens[i],"char")==0){//Bu 3 stringten birisi varsa o sat�rda deklare i�lemi yap�lcakt�r.
            start_index=i; //Deklare i�leminin ba�lang�� indisini(sat�r ba��) buras� yap�yoruz.
        }

        if(strchr(tokens[i],';')!=NULL){ //E�er tokenlerde ';' varsa sat�r sonuna geldik demektir.
            finish_index=i; // Deklare i�lemi bu sat�rda burada bitiyor demektir. �lgili tipte de�i�ken tan�mlanma i�leminin token dizisinde nerede bitti�ini bulduk.
            if(start_index!=0){// Ba�lang�� indisi 0 de�ilse yani declare i�lemi yap�l�yorsa
            	finish_declare=i; // T�m deklare i�lemleri token dizisinde nerede bitiyor onu anlamak i�in bu de�i�keni ayarlad�k.
                for(j=start_index+1;j<finish_index;j++){// Ba�lang�� indisi de�i�kenin tipini tutuyor. Ondan bir sonraki ba�layarak de�i�kenleri tabloya ekleyece�iz.
                    if(lookup(symbol_table,m,tokens[j])==0){// Tabloya daha �nce eklenmemi�se ekliyoruz.
                        insert(symbol_table,m,tokens[j],tokens[start_index]); //Tabloya ilgili de�i�keni ekliyoruz.
                    }
                    else{ //Eklenmi�se a�a��daki hatay� verip bir daha eklemiyoruz.
                    	printf("Hata: '%s' degiskeni daha once deklare edilmistir.\n",tokens[j]);
					}
                }
                // Sat�r sonundaki de�i�keni for i�inde eklemedik ��nk� sonunda ';' i�areti var yani key de�eri yanl�� hesaplanacak. Onu kald�rmam�z laz�m.
                size_t len=strlen(tokens[finish_index]);// Sat�r sonu de�i�keninin uzunlu�unu �l�eriz.
				tmp=malloc(len); // O uzunlukta tmp stringi olu�tururuz.
				if(tmp){ 
				    strncpy(tmp,tokens[finish_index],len - 1); // Uzunlu�unun bir eksi�ine kadar tmp atar�z. Yani asl�nda son karakter olan ';' kald�r�r�z.
				    tmp[len-1]='\0'; // tmp string oldu�u i�in sonuna Null terminat�r� eklemeliyiz.
				}
                if(lookup(symbol_table,m,tmp)==0) { //Bu de�i�ken de tabloda var m� diye kontrol yapar�z.
                    insert(symbol_table,m,tmp,tokens[start_index]);//Tabloya ilgili de�i�ken �nceden olmad��� i�in ekliyoruz.
                }else{ // Daha �nce tabloda bu de�i�ken varsa eklememeliyiz.
                	printf("Hata: '%s' degiskeni daha once deklare edilmistir.\n", tmp);
				}
                free(tmp);
                start_index=0; // Deklare i�leminin sat�r ba�lar�n� bulabilmek i�in 0 yapar�z. 0 de�ilse o sat�rda deklare i�lemi yap�l�yor. 0 ise deklare i�lemi yok. Genel i�lemler yap�lmaktad�r.
            }
        }
        i++;
    }
    
    finish_declare++; //Bu de�i�ken en son deklare edilen de�i�kenin token dizisinde yerini tutuyordu. Sonras�nda deklare i�lemi olmad��� i�in bir art�rmal�y�z.
    //Declare k�sm� kontrol bitti. Kod kontrol� yap�caz.
	while(finish_declare<token_count){// Deklare i�lemleriyle i�imiz bitti. Burada genel i�lemlere bakaca��z.
		if(strchr(tokens[finish_declare],'_') != NULL){ // �lgili token '_' i�areti bar�nd�r�yorsa bu de�i�kendir.
			if(strchr(tokens[finish_declare],';') != NULL){ // �lgili token ';' i�areti de bar�nd�r�yorsa bu sat�r sonundad�r. Tokenin sonunda ';' var.Kald�rmak gerekiyor.
				size_t len=strlen(tokens[finish_declare]);//�stte birebir ayn�lar�n� kullanm��t�k. token sonundaki ';' i�aretini kald�r�r�z.
				tmp=malloc(len);
				if(tmp){
				    strncpy(tmp,tokens[finish_declare],len - 1);
				    tmp[len-1]='\0'; 
				}
				if(lookup(symbol_table,m,tmp)==0){ //Tabloda daha �nce eklenmemi�se bu de�i�ken declare k�sm�nda da de�il. Bunun anlam� daha �nce tan�mlanmad���d�r.
					printf("Hata: '%s' degiskeni deklere edilmemistir.\n", tmp);
				}
			}else{// De�i�ken sat�r sonunda de�il. Yani ';' bar�nd�rm�yo direkt i�lem yapabiliriz.s
				if(lookup(symbol_table,m,tokens[finish_declare])==0){ //Tabloda yoksa daha �nce tan�mlanmam�� demektir.
				printf("Hata: '%s' degiskeni deklere edilmemistir.\n", tokens[finish_declare]);
			}
			}	
		}
	finish_declare++; 
	}
}

/*
@brief token dizisini serbest b�rak�r.
@param tokens token dizisi
@param token_count token say�s�
*/
void freeTokens(char **tokens,int token_count){ 
    int i;
    for(i=0;i<token_count;i++) {
        free(tokens[i]);
    }
    free(tokens);
}

/*
@brief Debug modu �al��t�r�r ve gerekli hatalar� ve de�i�kenleri ekranda g�sterir.
@param symbol_table sembol tablosu
@param m tablo uzunlu�u
*/
void debugMode(Symbol *symbol_table,int m){
	int i,variable_count=0,idx,key;
	printf("\n");
	printf("Tablonun son goruntusu:\n");
	for(i=0;i<m;i++){
		if(symbol_table[i].flag==1){
			variable_count++;
		}
		printf("%d %s %s\n",i,symbol_table[i].type,symbol_table[i].name);
	}
	printf("\n");
	printf("Deklere edilmis degisken sayisi:%d\nSembol tablosunun uzunlugu:%d\n",variable_count,m);
	printf("\n");
	for(i=0;i<m;i++){
		if(symbol_table[i].flag==1){
			key = hornerKey(symbol_table[i].name);
        	idx = h(key,0,m);
        	printf("%s degiskeninin hashtable uzerinde hesaplanan ilk adresi:%d  Sonda yerlestirilen adresi:%d\n",symbol_table[i].name,idx,i);
		}
	}
}

//Kod default olarak normal modda �al���yor. Kullan�c� debug modu se�erse normal modun �zerine debug �al���yor.
int main() {
    int token_count=0,m = 0,mode;
    char filename[50];
    Symbol *symbol_table;
    printf("Dosyanizin ismini giriniz: ");
    scanf("%s",filename);
	printf("Normal mode icin 1, debug mode icin 2 giriniz.\n");
	do{
		scanf("%d",&mode);
	}while(mode!=1 && mode!= 2);	
    char **tokens=readFile(filename,&token_count);
    if (!tokens) return 1;
    symbol_table=createTable(tokens,token_count, &m);
    if (!symbol_table) return 1;
    insertTable(tokens,token_count,symbol_table,m);
	if(mode==2){
		debugMode(symbol_table,m);
	}
    freeTokens(tokens,token_count);
    free(symbol_table);
    return 0;
}

