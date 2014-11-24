#include <linux/kernel.h>

#include <linux/module.h>

#include <linux/fs.h>

#include <asm/uaccess.h>

#include <linux/types.h>

#include <linux/kdev_t.h>

#include <linux/cdev.h>

#include <linux/slab.h>

#include <linux/moduleparam.h>

#include <linux/init.h>



//Prototypy funkcji - powinny byc w pliku .h







static int __init chardev_init( void);

static void __exit chardev_exit( void);

static int device_open( struct inode *, struct file * );

static int device_release( struct inode *, struct file * );

static ssize_t device_read( struct file *, char *, size_t, loff_t *);

static ssize_t device_write( struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0

#define DEVICE_NAME "chardev"

//#define BUF_LEN 100



//module_param( BUF_LEN2,short, S_IRUSR|S_IWUSR|S_IWUSR|S_IRGRP|S_IWGRP);



/* Zmienne globalne sa statyczne, a przez to pozostaja globalne dla pliku */

//static char msg[ BUF_LEN ]; // Bufor na wiadomosc od modulu

//static int licznik = 0;

static int Major = 122; // Glowny numer urzadzenia w /dev

DEFINE_MUTEX( Device_Open); // Mutex zabezpieczajacy przed wyscigiem o zasoby

static char *msg_Ptr; // wskaznik do ostatnio odczytanego miejsca

//static char licznik2=0;

static char *osta;

static char *msg;



//static short size=10;

//module_param( size, short, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

//MODULE_PARM_DESC( size, "Rozmiar bufora dynamicznego, typu 'short'");



static struct cdev *chcdev;



/* Struktura file_operations jest zdefiniowana w linux/fs.h i zawiera wskaznik do roznych funkcji, ktore sterownik urzadzenia moze zdefiniowac, a ktore beda wywolywane w przeroznych sytuacjach.*/



static int device_open( struct inode *inoda, struct file *plik){

	/* Informacja o numerze pobocznym 

	Makra unsigned int iminor (struct inode *in) i 

	unsigned int imajor( struc inode *in) zwracaja numer glowny i poboczny pliku urzadzenia wskazywanego przez inode in */

	printk( KERN_INFO "Otwarcie pliku urzadzenia o numerze pobocznym" " %d\n", iminor( inoda) );

	// Czy ktos juz nie korzysta z urzadzenia? (proba przejecia semafora)

	if( mutex_lock_interruptible( &Device_Open) ){

		printk( KERN_INFO "Proba przejecia semafora przerwana!\n");

		return -ERESTARTSYS;/* Zwrocona wartosc rozna od zera oznacza przerwanie procesu uzytkownika. W takim przypadku semafor NIE 						zostal przejety */

	}



	/* Zwiekszenie licznika uzycia modulu, uniemozliwi jego usuniecie z jadra */

	try_module_get( THIS_MODULE);



	/* Zapisanie komunikatu, ktory zostanie przepisany do pamieci uzytkownika w momencie odczytu */

	//sprintf( msg, "hello, world! mowie po raz %d\n", ++licznik);

	if( (plik->f_flags & O_ACCMODE)==O_WRONLY){

		kfree(msg);

		osta=NULL;

		msg=NULL;

	}

	if( (plik->f_flags & O_ACCMODE)==O_RDONLY){

		msg_Ptr=msg;

		//licznik2=strlen(msg);

	}

	return SUCCESS;

}

static int device_release( struct inode *inoda, struct file *plik ){



	mutex_unlock(&Device_Open); // Odblokowanie dostepu

	/* Jesli nie zmniejszy sie licznika uzycia modulu nie da sie go usunac z jadra */

	module_put( THIS_MODULE);

	return 0;

}



static ssize_t device_read( struct file*plik, char *buforUz, size_t dlugosc, loff_t *offset)

{

	//buforUz to miejsce, gdzie nalezy wpisac odpowiedz,

	//ma on dlugosc miejsca	

	//Liczba bajtow tym razem wpisanych do bufora - calosc

	//wiadomosci niekoniecznie musza byc przeczytane za jednym razem	

	// Jesli osiagnieto koniec bufora, zwracamy 0 

	//Koniec jest rozpoznany, bo jest w nim znak konca lancucha znakowego	

	// Przepisanie danych do bufora uzytkowanika

		/* Poniewaz bufor jest w przestrzeni danych uzytkownika a nie w pamieci jadra, nie mozna danych przepisywac bezposrednio, a 			jedynie za pomoca funkcji put_user, ktora sluzy wlasnie do przepisania danych z pamieci jadra do pamieci uzytkownika */

/////////////////////////////////////////////////////////////////////////////////////////////////

	/*int odczytano = 0;

	while( dlugosc && licznik2){

		put_user( *(msg_Ptr++), buforUz++);

		dlugosc--;

		odczytano++;

		licznik2--;

		//osta=msg_Ptr;

	}

	*offset += odczytano;

	//Wiekszosc funkcji do odczytu zwraca ilosc przepisanych danych

	return odczytano;*/

///////////////////////////////////////////////////////////////////////////////////////////////////

	int amount,odczytano;//,remain;

	//remain=dlugosc;

	if(msg_Ptr==osta){

		return -1;

	}

	else {	

		amount=osta-msg_Ptr; //ilosc danych do odczytania

		if(amount>=dlugosc){

			amount=dlugosc;

			/*remain*/amount=copy_to_user(buforUz,msg_Ptr,amount);

			msg_Ptr=osta-amount;

			odczytano=osta-msg_Ptr;

		}

		else{

			odczytano=amount;

		}

	}

	return odczytano;	



}



/*int ile_zost=0;

int odczytano = 0;

int ile_odczytac=0;







if( msg_Ptr ==  koniec)                    ///////////////////////////////////////////////////

{

return -1;

}

else

{



ile_odczytac=koniec-msg_Ptr;

if(ile_odczytac<dlugosc)

    odczytano=ile_odczytac;

else

    odczytano=dlugosc;

    ile_zost=copy_to_user(buforUz,msg_Ptr,odczytano); //odczyt

    msg_Ptr+=odczytano-ile_zost;

    odczytano=odczytano-ile_zost;

}*/



static ssize_t device_write( struct file *plik, const char *bufor, size_t dlugosc, loff_t *offset)

{

	int zapisano =0;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//char *wiadomosc;

	//while(dlugosc && (zapisano<BUF_LEN2) )

	//{

//copy_from_user(msg,bufor,size);

//zapisano=strlen(*msg);

	//get_user( *(wiadomosc++),bufor++);

	//msg[zapisano]=wiadomosc;

	//dlugosc--;

	//zapisano++;

		//copy_from_user(*(msg++),bufor++,BUF_LEN2);

	//}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



	static char *msg2; //kolejny bufor, do powiekszania

	//static int tmp; //dlugosc

	//tmp=strlen(*msg);

	//	printk(KERN_INFO "Ilosc znakow do przepisania: %d", tmp);

	//int remain=tmp-zapisano;

	int remain=osta-msg_Ptr;

		printk(KERN_INFO "Ilosc znakow pozostala do przepisania: %d", remain);

	msg2=kmalloc((remain+dlugosc),GFP_KERNEL); //size	

	if(remain<=0){

		printk(KERN_INFO "Przepisano cala informacje");

		return -1;

	}

	else{

		if(msg==NULL){

			msg=msg2;

		}

		else{

			memcpy(msg2,msg,remain);////size)

			kfree(msg);

			msg=msg2;

		}

	}

	remain=copy_from_user(msg+remain,bufor,dlugosc);

	zapisano=dlugosc-remain;

	osta=osta+zapisano;

	*offset += zapisano;

	return zapisano;

	//printk( KERN_ALERT "To urzadzenie nie obsluguje zapisu!\n" );

	//return -EINVAL;

}



/*int tyle_bd,ile_zost,tyle_jest,odczytano=0;

tyle_jest=koniec-msg_Ptr;

tyle_bd=tyle_jest+dlugosc;

char *tymczasowy=kmalloc(tyle_bd,GFP_KERNEL);

if(tymczasowy==NULL)

    return -1;

if(msg!=NULL)

{

    memcpy(tymczasowy,msg,tyle_jest);

    kfree(msg);

    msg=tymczasowy;

}

else

{

    msg=tymczasowy; 

}

koniec=msg+tyle_bd;



ile_zost=copy_from_user(msg+tyle_jest,bufor,dlugosc); //zapis

odczytano=dlugosc-ile_zost;

koniec = msg + odczytano-ile_zost;*/

			





/*Ponizej zdefiniowano cztery najbardziej podstawowe z nich, korzystajac z rozszerzenia zdefiniowanego w standardzie C99*/



static struct file_operations fops = {

	.read = device_read,

	.write = device_write,

	.open = device_open,

	.release = device_release

};

static int __init chardev_init( void)

{

	int ret;

	dev_t num;

	//msg=kmalloc(size,GFP_KERNEL);

	//if (msg==NULL)

	//	{

	//	printk( KERN_ALERT "Nieudana proba przydzialu obszaru urzadzenia");

	//	}

	/* w jadrach od 2.6.0 zmienila sie numeracja urzadzen. Typ dev_t jest obecnie liczba 32-bitowa, ktorej pierwsze 12 bitow 		okresla Major, a reszta Minor. Do zbudowania typu dev_t z Major i Minor sluzy ponizsze makro: */

	num = MKDEV( Major, 0);

	/* Rejestracja obszaru urzadzen w jadrze. Pierwszym parametrem funkcji jest pierwszy z numerow zadanego obszaru (jego czesc Minor 	najczesciej jest rowna zero, ale nie jest wymog formalny). Drugim jest liczba numerow, ktore chce sie zarejestrowac. Trzecim jest nazwa.

	Wartosc zwracana niezerowa oznacza blad przydzialu urzadzenia */

	ret = register_chrdev_region( num, 3, DEVICE_NAME);

	if( ret < 0){

		printk( KERN_ALERT "Nieudana proba przydzialu obszaru urzadzenia" " w jadrze - zwrocony numer %d\n", ret );

		return ret;

	}

	/* ponizsze instrukcje sluza do dynamicznej alokacji struktury cdev potrzebnej do rejestracji urzadzenia znakowego w jadrze > 2.6.0 	oraz do inicjalizacji jej najwazniejszych pol */

	chcdev = cdev_alloc();

	chcdev->owner = THIS_MODULE;

	chcdev->ops = &fops;

	/* Rejestracja urzadzenia znakowego w jadrze. Pierwszym parametrem jest wskaznik do struktury cdev, drugim numer ( w postaci dev_t) 		poczatku obszaru, za ktory odpowiedzialnosc jest zapisana w cdev, trzecim rozmiar. Wartosc zwrocona mniejsza od zera oznacza blad 		rejestracji. */

	ret = cdev_add( chcdev, num, 3);

	if ( ret<0){

		printk( KERN_ALERT "Nieudana proba zarejestrowania urzadzenia" "w jadrze - zwrocony numer %d\n", ret );

		return ret;

	}



	//Komunikat do logow systemowych

	printk( KERN_INFO "Przydzielono mi numer urzadzenia %d. Otworz plik\n", Major );

	printk( KERN_INFO "urzadzenia za pomoca"

			"'mknod /dev/%s c %d 0', a potem\n", DEVICE_NAME, Major);

	printk( KERN_INFO "z inna ostatnia cyfra. Probuj czytac i pisac do tego\n" );

	printk( KERN_INFO "urzadzenia. Po usunieciu urzadzenia usun i plik\n");



	return SUCCESS;

}



static void __exit chardev_exit( void)

{

	dev_t num;

	kfree(msg);

	/* w jadrach od 2.6.0 zmienila sie numeracja urzadzen. Typ dev_t jest obecnie liczba 32-bitowa, ktorej pierwsze 12 bitow okresla 	Major, a reszta Minor. DO zbudowania typu dev_t z Major i Minor sluzy ponizsze makro: */

	num = MKDEV( Major, 0);

	/* Funkcja wyrejestrujaca urzadzenie znakowe z jadra (zwraca void) */

	cdev_del( chcdev);

	/* Funkcja zwalniajaca zarezerwowany obszar urzadzenia (zwraca void) */

	unregister_chrdev_region( num, 3);

	printk( KERN_INFO "Zegnaj, swiecie!\n");

}



module_init( chardev_init);

module_exit( chardev_exit);



MODULE_LICENSE("GPL v2");

//MODULE_LICENSE("Proprietary");

/* Autor modulu */

MODULE_AUTHOR( "Dariusz Biszmor <Dariusz.Bismor[at]polsl.pl>");

/*k Krotki opis modulu */

MODULE_DESCRIPTION( "Przyklad modulu z plikiem urzadzenia" );

/* Modulu uzywa urzadzenia /dev/test */

MODULE_SUPPORTED_DEVICE( DEVICE_NAME);









