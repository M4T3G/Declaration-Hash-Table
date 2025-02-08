#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NAME_LEN 100

//Hashtable için struct yapýsý
typedef struct{
    char name[MAX_NAME_LEN];
    char type[10];
    int flag; // 0: boþ, 1: dolu
}Symbol;

/*
@brief Sayýnýn asal olup olmadýðýný kontrol eder
@param n Asallýðý kontrol edilecek sayý
@return 1 ile dönerse sayý asaldýr. 0 ile dönerse sayý asal deðildir.
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
@brief Verilen sayýdan sonraki asal sayýyý bulur.
@param n kendisinden sonraki asal sayý bulunmasý istenen sayý
@return Sonraki asal sayý döndürülür.
*/
int nextPrime(int n){
    while(!isPrime(n))n++;
    return n;
}

/*
@brief Tabloya yerleþtirilecek deðiþkenlerin isimlerini hash fonksiyonuna verebilmek için sayý karþýlýklarý Horner methodu ile hesaplanýr.
@param str horner methodu ile keyi hesaplanacak deðiþken
@return Sayýsal deðeri hesaplanmýþ deðiþken
*/
int hornerKey(char *str){
    int key=0,i;
    for (i=0;str[i]!='\0';i++){
        key=key*31+str[i]; // Horner kuralý
    }
    return key;
}

/*
@brief ilk hash fonksiyonu
@param key horner methoduyla hesaplanmýþ deðer
@param m tablo uzunluðu
*/
int h1(int key,int m){
    return key % m;
}

/*
@brief ikinci hash fonksiyonu
@param key horner methoduyla hesaplanmýþ deðer
@param m tablo uzunluðu
*/
int h2(int key,int m) {
    return 1+(key%(m-3));
}

/*
@brief iki hash fonksiyonunu kullanarak deðiþkenin tabloda yerleþecek yerini bulur.
@param key horner methoduyla hesaplanmýþ deðer
@param i tablodaki ilgili yer doluysa double hash ile sonraki boþ yeri bulmaya yarar.
@param m tablo uzunluðu
*/
int h(int key,int i,int m){
    return (h1(key, m)+i*h2(key, m))%m;
}

/*
@brief Dosyadan string deðerlerini okur ve token olarak her bir stringi ayýrýr.
@param filename açýlacak dosyanýn ismi
@param token_count açýlacak dosyada token yani string sayýsýný saklar.
@return tokens parçalanmýþ stringleri tutan string dizisinin adresini döndürür.
*/
char **readFile(char *filename,int *token_count){
    FILE *file=fopen(filename, "r");
    if(!file){
        printf("Hata: Dosya açýlamadý.\n");
        return NULL;
    }

    char line[256];
    char **tokens=malloc(100*sizeof(char *)); // Token dizisi için bellek ayýr
    *token_count=0;

    while(fgets(line,sizeof(line),file)){    // Satýr satýr dosyayý okur ve stringleri verilen iþaretleri kaldýrarak ve ona göre ayýrarak token dizisine atar.
        char *token = strtok(line, " <>\t=%+-/[]{},()\n");  // Noktalý virgülü kaldýrýlacaklar dizisine eklemedim ona göre iþlem yapacaðým.
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
@brief Token dizisini yani parçalanmýþ stringleri ekrana yazdýrýr.
@param tokens token dizisi 
@param token_count toplam token sayýsý
*/
void printTokens(char **tokens,int token_count){
    int i;
    for (i=0;i<token_count;i++) {
        printf("Token %d: %s\n",i+1,tokens[i]);
    }
}

/*
@brief  Tokenleri gezerek, tanýmlanmaya çalýþan deðiþken sayýsýný bularak onun boyutu kadar uzunlukta sembol tablosunu oluþturur.
@param tokens token dizisi
@param token_count token sayýsý
@param m tablonun uzunluðu
@return sembol tablosu yani hashtable
*/
Symbol *createTable(char **tokens,int token_count,int *m){
    int i=0,variable_count=0,start_index=0,finish_index=0;
    Symbol *symbol_table;

    while(i<token_count){
        if (strcmp(tokens[i],"int")== 0 || strcmp(tokens[i],"float")==0 || strcmp(tokens[i], "char")==0){ //Bu 3 stringten birisi varsa o satýrda deklare iþlemi yapýlcaktýr.
            start_index = i; //Deklare yapýlacak token indisini sakladýk.
        }
        if(strchr(tokens[i],';')!=NULL){ // ';' içeren token varsa
            finish_index=i; //Satýr sonuna geldik demektir. Start indisindeki tipte deðiþkenler deklare edildiðini anlýyoruz.
            if(start_index>0){ //Start indisi 0'dan büyükse yani deðiþken tanýmlanýyorsa 
                variable_count+=(finish_index-start_index);// Ýlgili satýrda kaç deðiþken tanýmlandýðýný bu þekilde bulabiliriz.
                start_index=0; //Sonrasýnda deklare iþlemi yoksa variable_count sayýsýna etki etmesin ve bu if'e girmesin diye 0 yapýyoruz. 
				//Deklare iþlemi yapýlýyorsa bu tekrardan 0'dan farklý bir sayý olur zaten. 
            }
        }
        i++;
    }

    *m=nextPrime(variable_count*2);  //Ýki katýndan sonraki asal sayýsý kadar tablo boyutu oluþturuyoruz.
    symbol_table=(Symbol *)calloc(*m, sizeof(Symbol));
    if (!symbol_table){
        printf("Hata: Bellek tahsis edilemedi.\n");
        return NULL;
    }
    return symbol_table;
}

/*
@brief Tablonun ilgili gözü boþsa oraya gerekli deðiþkeni ekler.
@param table sembol yani hashtable
@param m tablo uzunluðu
@param name eklenmesi istenilen deðiþkenin ismi
@param type eklenmesi istenilen deðiþkenin tipi
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
@brief tabloda parametre olarak verilen deðiþkenin önceden tabloya eklenip eklenmediðini kontrol eder.
@param table sembol tablosu
@param m tablo uzunluðu
@param name tabloda var mý diye kontrol edilmesi istenen deðiþken
@return Eðer tabloda o deðerin aynýsý önceden eklenmiþse 1 eklenmemiþse 0 deðerini döndürür.
*/
int lookup(Symbol *table,int m,char *name){
	int i=0;
    int key=hornerKey(name);
    while(i<m){  //Tabloyu gezer.
        int idx=h(key, i, m);
        if(strcmp(table[idx].name, name)==0){  //Ayný isimde deðiþken var mý diye kýyaslar.
        	return 1;
		}      
        i++;
    }
    return 0;
}

/*
@brief Tabloya ekleme ve hatalarý kontrol iþleri burada olur.
@param tokens token dizisi
@param token_count token sayýsý
@param symbol_table sembol tablosu
@param m tablo uzunluðu
*/
void insertTable(char **tokens,int token_count,Symbol *symbol_table,int m){
    int i=0,j,start_index = 0,finish_index = 0,finish_declare=0;
    char *tmp;

    while(i<token_count){  //Variable sayýsý hesaplamada yaptýðýmýz iþleme benzer bir þekilde iþlem yapacaðýz.
        if(strcmp(tokens[i],"int")==0 || strcmp(tokens[i],"float")==0 || strcmp(tokens[i],"char")==0){//Bu 3 stringten birisi varsa o satýrda deklare iþlemi yapýlcaktýr.
            start_index=i; //Deklare iþleminin baþlangýç indisini(satýr baþý) burasý yapýyoruz.
        }

        if(strchr(tokens[i],';')!=NULL){ //Eðer tokenlerde ';' varsa satýr sonuna geldik demektir.
            finish_index=i; // Deklare iþlemi bu satýrda burada bitiyor demektir. Ýlgili tipte deðiþken tanýmlanma iþleminin token dizisinde nerede bittiðini bulduk.
            if(start_index!=0){// Baþlangýç indisi 0 deðilse yani declare iþlemi yapýlýyorsa
            	finish_declare=i; // Tüm deklare iþlemleri token dizisinde nerede bitiyor onu anlamak için bu deðiþkeni ayarladýk.
                for(j=start_index+1;j<finish_index;j++){// Baþlangýç indisi deðiþkenin tipini tutuyor. Ondan bir sonraki baþlayarak deðiþkenleri tabloya ekleyeceðiz.
                    if(lookup(symbol_table,m,tokens[j])==0){// Tabloya daha önce eklenmemiþse ekliyoruz.
                        insert(symbol_table,m,tokens[j],tokens[start_index]); //Tabloya ilgili deðiþkeni ekliyoruz.
                    }
                    else{ //Eklenmiþse aþaðýdaki hatayý verip bir daha eklemiyoruz.
                    	printf("Hata: '%s' degiskeni daha once deklare edilmistir.\n",tokens[j]);
					}
                }
                // Satýr sonundaki deðiþkeni for içinde eklemedik çünkü sonunda ';' iþareti var yani key deðeri yanlýþ hesaplanacak. Onu kaldýrmamýz lazým.
                size_t len=strlen(tokens[finish_index]);// Satýr sonu deðiþkeninin uzunluðunu ölçeriz.
				tmp=malloc(len); // O uzunlukta tmp stringi oluþtururuz.
				if(tmp){ 
				    strncpy(tmp,tokens[finish_index],len - 1); // Uzunluðunun bir eksiðine kadar tmp atarýz. Yani aslýnda son karakter olan ';' kaldýrýrýz.
				    tmp[len-1]='\0'; // tmp string olduðu için sonuna Null terminatörü eklemeliyiz.
				}
                if(lookup(symbol_table,m,tmp)==0) { //Bu deðiþken de tabloda var mý diye kontrol yaparýz.
                    insert(symbol_table,m,tmp,tokens[start_index]);//Tabloya ilgili deðiþken önceden olmadýðý için ekliyoruz.
                }else{ // Daha önce tabloda bu deðiþken varsa eklememeliyiz.
                	printf("Hata: '%s' degiskeni daha once deklare edilmistir.\n", tmp);
				}
                free(tmp);
                start_index=0; // Deklare iþleminin satýr baþlarýný bulabilmek için 0 yaparýz. 0 deðilse o satýrda deklare iþlemi yapýlýyor. 0 ise deklare iþlemi yok. Genel iþlemler yapýlmaktadýr.
            }
        }
        i++;
    }
    
    finish_declare++; //Bu deðiþken en son deklare edilen deðiþkenin token dizisinde yerini tutuyordu. Sonrasýnda deklare iþlemi olmadýðý için bir artýrmalýyýz.
    //Declare kýsmý kontrol bitti. Kod kontrolü yapýcaz.
	while(finish_declare<token_count){// Deklare iþlemleriyle iþimiz bitti. Burada genel iþlemlere bakacaðýz.
		if(strchr(tokens[finish_declare],'_') != NULL){ // Ýlgili token '_' iþareti barýndýrýyorsa bu deðiþkendir.
			if(strchr(tokens[finish_declare],';') != NULL){ // Ýlgili token ';' iþareti de barýndýrýyorsa bu satýr sonundadýr. Tokenin sonunda ';' var.Kaldýrmak gerekiyor.
				size_t len=strlen(tokens[finish_declare]);//Üstte birebir aynýlarýný kullanmýþtýk. token sonundaki ';' iþaretini kaldýrýrýz.
				tmp=malloc(len);
				if(tmp){
				    strncpy(tmp,tokens[finish_declare],len - 1);
				    tmp[len-1]='\0'; 
				}
				if(lookup(symbol_table,m,tmp)==0){ //Tabloda daha önce eklenmemiþse bu deðiþken declare kýsmýnda da deðil. Bunun anlamý daha önce tanýmlanmadýðýdýr.
					printf("Hata: '%s' degiskeni deklere edilmemistir.\n", tmp);
				}
			}else{// Deðiþken satýr sonunda deðil. Yani ';' barýndýrmýyo direkt iþlem yapabiliriz.s
				if(lookup(symbol_table,m,tokens[finish_declare])==0){ //Tabloda yoksa daha önce tanýmlanmamýþ demektir.
				printf("Hata: '%s' degiskeni deklere edilmemistir.\n", tokens[finish_declare]);
			}
			}	
		}
	finish_declare++; 
	}
}

/*
@brief token dizisini serbest býrakýr.
@param tokens token dizisi
@param token_count token sayýsý
*/
void freeTokens(char **tokens,int token_count){ 
    int i;
    for(i=0;i<token_count;i++) {
        free(tokens[i]);
    }
    free(tokens);
}

/*
@brief Debug modu çalýþtýrýr ve gerekli hatalarý ve deðiþkenleri ekranda gösterir.
@param symbol_table sembol tablosu
@param m tablo uzunluðu
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

//Kod default olarak normal modda çalýþýyor. Kullanýcý debug modu seçerse normal modun üzerine debug çalýþýyor.
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

