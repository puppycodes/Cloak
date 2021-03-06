/*******************************************************************************
**
** Name:        main.cpp
**
** Author:      Guy Wilson - Nov 2016
**
** Licence:     This program may be used freely for any purpose with no warranty
**              of any kind.
**
** Description: The idea here is simple, take a 24-bit RGB bitmap or PNG,
**              encode within the image another file you wish to hide.
**              By encoding the file in the LSB of the image bytes, 
**				there will be no noticeable difference to the image itself, 
**				as we are only (maybe) changing the least significant bit (LSB) 
**              of each byte when encoding with 1-bit per byte. Encoding with
**				4-bits per byte will introduce a noticeable 'grain' effect on some
**				images.
**              This is only effective for true colour bitmaps (24 bit) as other 
**				bitmaps use a colour palette.
**
*******************************************************************************/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#endif

extern "C" {
	#include "memdebug.h"
}

#include "secure_func.h"
#include "cloak.h"
#include "exception.h"
#include "errorcodes.h"
#include "main.h"

using namespace std;

#ifndef _WIN32
int _strcmpi(const char *pszStr, const char *pszCompare)
{
	int		i;
	int		len;
	
	if (strlen(pszStr) != strlen(pszCompare)) {
		return -1;
	}
	
	len = strlen(pszStr);
	
	for (i = 0;i < len;i++) {
		if (toupper(pszStr[i]) != toupper(pszCompare[i])) {
			return -1;
		}
	}
	
	return 0;
}
#endif

int main(int argc, char *argv[])
{
	char			szCmd[512];
    bool			bContinueCmdLoop;
	
    if (argc > 1) {
    	processParams(argc, argv);
    }
    else {
		bContinueCmdLoop = true;
	
		while (bContinueCmdLoop) {
			cout << "cloak> ";
			cin.getline(szCmd, 512);
		
			bContinueCmdLoop = processCommand(szCmd);
		}
    }
	
    return 0;
}

void printUsage()
{
    cout << "Usage:" << endl;
    cout << "    cloak -a/e -v -i [infile] -o [outfile] -s [secretfile] -k (keystream file) -b [bits per byte]" << endl << endl;
    cout << "    Where: -a = 'add' mode to add 'secretfile' to 'infile'" << endl;
    cout << "           -e = 'extract' mode to extract 'outfile' from 'infile'" << endl;
    cout << "           -v = 'verbose' mode, print bitmap header info" << endl;
    cout << "           -bn = Bits per byte, must be 1, 2 or 4" << endl;
    cout << "           infile  = an input bitmap" << endl;
    cout << "           outfile = output bitmap (add mode) or extracted file" << endl;
    cout << "           secretfile = secret input file to add to 'infile'" << endl << endl;
	cout.flush();
}

void printHeaderInfo(Cloak *cloak)
{
    int		i;
	byte	*pReserved;
	Image	*img;
	Bitmap  *bmp;
	
	img = cloak->getSourceImage();
	
	if (cloak->getSourceImageType() == rgb_bitmap) {
		bmp = (Bitmap *)img;
		cout << "File Size = " << bmp->getFileSize() << endl;
		cout << "Reserved char *= '";
		pReserved = bmp->getReserved();
		for (i = 0;i < 4;i++) {
			cout << pReserved[i];
		}
		cout << endl;
		cout << "Start Offset = 0x" << hex << bmp->getStartOffset() << endl;
		cout << "Header Size = " << bmp->getHeaderSize() << endl;
	}
    cout << "Width = " << img->getWidth() << endl;
    cout << "Height = " << img->getHeight() << endl;
    cout << "Bits per Pixel = " << img->getBitsPerPixel() << endl;
    cout << "Bitmap Data Length = " << img->getImageDataLength() << endl;
	cout << "Bitmap cloaking capacity = " << img->getCapacity(cloak->getBitsPerByte()) << endl;
}

void processParams(int argc, char *argv[])
{
    char            szInputFileName[FILENAME_BUFFER_LENGTH];
    char            szOutputFileName[FILENAME_BUFFER_LENGTH];
    char            szSecretFileName[FILENAME_BUFFER_LENGTH];
	char			szKeyFileName[FILENAME_BUFFER_LENGTH];
	int				keyMode = KEY_PASSWD;
	char			szPassword[128];
    int             mode = MODE_INFO_ONLY;
    int				i;
    bool         	bPrintInfo = false;
	word			usBitsPerByte;
	byte *			bKeyStream;
	dword			ulKeyLength;

    for (i = 1;i < argc;i++) {
        if (argv[i][0] == '-' || argv[i][0] == '/') {
            switch (argv[i][1]) {
                case '?':
                case 'h':
                    printUsage();
                    return;

                case 'a':
                    mode = MODE_ADD;
                    break;

                case 'e':
                    mode = MODE_EXTRACT;
                    break;

                case 'c':
                    mode = MODE_COPY;
                    break;

                case 'v':
                    bPrintInfo = true;
                    break;

                case 'i':
                    // Image file name...
                    strcpy_s(szInputFileName, FILENAME_BUFFER_LENGTH, argv[++i]);
                    break;

                case 's':
                    // 'Secret' file name...
                    strcpy_s(szSecretFileName, FILENAME_BUFFER_LENGTH, argv[++i]);
                    break;

                case 'o':
                    // Output file name (image/extracted file)...
                    strcpy_s(szOutputFileName, FILENAME_BUFFER_LENGTH, argv[++i]);
                    break;

                case 'k':
                    // Key file name (keystream)...
                    strcpy_s(szKeyFileName, FILENAME_BUFFER_LENGTH, argv[++i]);
					keyMode = KEY_STREAM;
                    break;

				case 'b':
                    // bits per byte...
                    usBitsPerByte = (byte)atoi(&argv[i][2]);
                    break;
            }
        }
    }

	if (mode != MODE_COPY) {
		if (usBitsPerByte != 1 && usBitsPerByte != 2 && usBitsPerByte != 4) {
			cout << "\nBits per byte must be specified as 1, 2 or 4" << endl << endl;
			return;
		}

		if (keyMode == KEY_PASSWD) {
			try {
				if (getPassword(szPassword, 64) != 0) {
					return;
				}
			}
			catch (Exception *e) {
				cout << e->getExceptionString() << endl;
				delete e;
				return;
			}
		}
		else if (keyMode == KEY_STREAM) {
			bKeyStream = getKeyStream(szKeyFileName, &ulKeyLength);
		}
	}
	else {
		usBitsPerByte = 1;
	}

	Cloak *cloak = new Cloak();

	cloak->setBitsPerByte(usBitsPerByte);

	if (mode == MODE_ADD) {
		try {
			cloak->loadSourceImage(szInputFileName);

			if (bPrintInfo) {
				printHeaderInfo(cloak);
			}
		
			cloak->loadSourceDataFile(szSecretFileName);
		
			if (keyMode == KEY_PASSWD) {
				cloak->merge(szOutputFileName, szPassword);
			}
			else if (keyMode == KEY_STREAM) {
				cloak->merge(szOutputFileName, bKeyStream, ulKeyLength);
			}
		}
		catch (Exception *e) {
			cout << e->getExceptionString() << endl;
			delete e;
			delete cloak;
			return;
		}
	}
	else if (mode == MODE_EXTRACT) {
		try {
			cloak->loadSourceImage(szInputFileName);

			if (bPrintInfo) {
				printHeaderInfo(cloak);
			}
		
			if (keyMode == KEY_PASSWD) {
				cloak->extract(szOutputFileName, szPassword);
			}
			else if (keyMode == KEY_STREAM) {
				cloak->extract(szOutputFileName, bKeyStream, ulKeyLength);
			}
		}
		catch (Exception *e) {
			cout << e->getExceptionString() << endl;
			delete e;
			delete cloak;
			return;
		}
	}
	else if (mode == MODE_COPY) {
		try {
			cloak->loadSourceImage(szInputFileName);

			if (bPrintInfo) {
				printHeaderInfo(cloak);
			}
		
			cloak->copy(szOutputFileName);
		}
		catch (Exception *e) {
			cout << e->getExceptionString() << endl;
			delete e;
			delete cloak;
			return;
		}
	}

	delete cloak;
}

bool processCommand(char *pszCommand)
{
	char			szImageFilename[FILENAME_BUFFER_LENGTH];
	char			szSecretFilename[FILENAME_BUFFER_LENGTH];
	char			szKeyFilename[FILENAME_BUFFER_LENGTH];
	char			szPassword[PASSWORD_BUFFER_LENGTH];
	byte *			pKeyStream;
	dword			ulKeyLength;
	static Cloak *	cloak = new Cloak();
	
	if (_strcmpi(pszCommand, "help") == 0 || _strcmpi(pszCommand, "?") == 0) {
		cout << "Welcome to Cloak interactive mode!" << endl << endl;
		cout << "Commands suported are:" << endl;
		cout << "    help (h)          What you are reading now." << endl;
		cout << "    load image (li)   Load an input image file." << endl;
		cout << "    load file (lf)    Load an input data file for cloaking." << endl;
		cout << "    merge (m)         Merge the input data file to the image." << endl;
		cout << "    extract file (ef) Extract a cloaked file from the input image." << endl;
		cout << "    copy (c)          Copy the input image to an output image." << endl;
		cout << "    set bits (sb)     Set the bits per byte for cloaking." << endl;
		cout << "    quit (exit, q)    Leave cloak interactive mode." << endl << endl;
		cout << "Please note, you will be prompted for a password with both 'merge' and" << endl;
		cout << "'extract file' commands, not entering a password (just hit enter) will" << endl;
		cout << "not encrypt the file. Entering a password will encrypt the file twice" << endl;
		cout << "with AES before cloaking it within the image. Cloak will not tell you" << endl;
		cout << "if you have the password wrong when extracting a file (it won't know" << endl;
		cout << "what it is) you will just end up with nonsense in your extracted file." << endl;
		cout << "It is unlikely that anyone (or any government agency) will be able to" << endl;
		cout << "prove that a given image contains an encrypted file, unless they know" << endl;
		cout << "the key of course. If you enter a keystream filename in 'merge' or" << endl;
		cout << "'extract file' mode, cloak will use the data in the keystream file" << endl;
		cout << "to encrypt the file instead (it won't prompt for a password)." << endl;
		cout << "Encoding with 1-bit per byte has the least impact on viewed image" << endl;
		cout << "quality but with the least storage capacity, 4-bits per byte will" << endl;
		cout << "give a noticeable grain effect to most images. An 800 x 800 image" << endl;
		cout << "can store a file size of approx. 230Kb at 1-bit per byte." << endl;
		cout << "Good Luck!" << endl;
	}
	else if (_strcmpi(pszCommand, "load image") == 0 || _strcmpi(pszCommand, "li") == 0) {
		try {
			cout << "Enter input image filename: ";
			cin.getline(szImageFilename, FILENAME_BUFFER_LENGTH);
		
			cloak->loadSourceImage(szImageFilename);
		}
		catch (Exception *e) {
			cout << e->getExceptionString() << endl;
			delete e;
			return 0;
		}
	}
	else if (_strcmpi(pszCommand, "load file") == 0 || _strcmpi(pszCommand, "lf") == 0) {
		try {
			cout << "Enter input filename: ";
			cin.getline(szSecretFilename, FILENAME_BUFFER_LENGTH);
		
			cloak->loadSourceDataFile(szSecretFilename);
		}
		catch (Exception *e) {
			cout << e->getExceptionString() << endl;
			delete e;
			return 0;
		}
	}
	else if (_strcmpi(pszCommand, "merge") == 0 || _strcmpi(pszCommand, "m") == 0) {
		try {
			if (cloak->getBitsPerByte() == 0) {
				getBitsPerByte(cloak);
			}
			
			cout << "Enter output image filename: ";
			cin.getline(szImageFilename, FILENAME_BUFFER_LENGTH);
			
			cout << "Enter keystream filename (Enter = none): ";
			cin.getline(szKeyFilename, FILENAME_BUFFER_LENGTH);
		
			if (strlen(szKeyFilename) == 0) {
				if (getPassword(szPassword, PASSWORD_BUFFER_LENGTH - 1) != 0) {
					return false;
				}
			
				cloak->merge(szImageFilename, szPassword);
			}
			else {
				pKeyStream = getKeyStream(szKeyFilename, &ulKeyLength);
				cloak->merge(szImageFilename, pKeyStream, ulKeyLength);
			}
		}
		catch (Exception *e) {
			cout << e->getExceptionString() << endl;
			delete e;
			return false;
		}
	}
	else if (_strcmpi(pszCommand, "extract file") == 0 || _strcmpi(pszCommand, "ef") == 0) {
		try {
			if (cloak->getBitsPerByte() == 0) {
				getBitsPerByte(cloak);
			}
			
			cout << "Enter extracted filename: ";
			cin.getline(szSecretFilename, FILENAME_BUFFER_LENGTH);
			
			cout << "Enter keystream filename (Enter = none): ";
			cin.getline(szKeyFilename, FILENAME_BUFFER_LENGTH);
			
			if (strlen(szKeyFilename) == 0) {
				if (getPassword(szPassword, PASSWORD_BUFFER_LENGTH - 1) != 0) {
					return false;
				}
			
				cloak->extract(szSecretFilename, szPassword);
			}
			else {
				pKeyStream = getKeyStream(szKeyFilename, &ulKeyLength);
				cloak->extract(szSecretFilename, pKeyStream, ulKeyLength);
			}
		}
		catch (Exception *e) {
			cout << e->getExceptionString() << endl;
			delete e;
			return false;
		}
	}
	else if (_strcmpi(pszCommand, "copy") == 0 || _strcmpi(pszCommand, "c") == 0) {
		try {
			cout << "Enter output image filename: ";
			cin.getline(szImageFilename, FILENAME_BUFFER_LENGTH);
		
			cloak->copy(szImageFilename);
		}
		catch (Exception *e) {
			cout << e->getExceptionString() << endl;
			delete e;
			return false;
		}
	}
	else if (_strcmpi(pszCommand, "set bits") == 0 || _strcmpi(pszCommand, "sb") == 0) {
		try {
			getBitsPerByte(cloak);
		}
		catch (Exception *e) {
			cout << e->getExceptionString() << endl;
			delete e;
			return false;
		}
	}
	else if (_strcmpi(pszCommand, "quit") == 0 || _strcmpi(pszCommand, "exit") == 0 || _strcmpi(pszCommand, "q") == 0) {
		cout << "bye..." << endl;
		delete cloak;
		return false;
	}
	
	return true;
}

byte * getKeyStream(char *pszKeyFilename, dword * ulKeyLength)
{
	byte *		keyStream;
	
	DataFile * keyFile = new DataFile(pszKeyFilename);
	
	keyFile->read();
	
	keyStream = keyFile->getData();
	
	*ulKeyLength = keyFile->getFileLength();
	
	return keyStream;
}

void getBitsPerByte(Cloak *cloak)
{
	char	szBitsPerByte[16];
	word	bitsPerByte;
	
	cout << "Enter bits per byte (1, 2, or 4): ";
	cin.getline(szBitsPerByte, 16);
	
	bitsPerByte = (word)atoi(szBitsPerByte);
	
	if (bitsPerByte == 1 || bitsPerByte == 2 || bitsPerByte == 4) {
		cloak->setBitsPerByte((word)atoi(szBitsPerByte));
	}
	else {
		throw new Exception(ERR_VALIDATION, "Bits per byte must be specified as 1, 2 or 4");
	}
}

int getPassword(char *pszPassword, int maxLen)
{
	char	*pszPassword1;
	char	*pszPassword2;
	
	pszPassword1 = (char *)malloc_d(maxLen + 1, "main:getPassword():pszPassword1");
	
	if (pszPassword1 == NULL) {
		throw new Exception(ERR_MALLOC, "Failed to allocate memory for password", __FILE__, "main", "getPassword", __LINE__);
	}
	
	pszPassword2 = (char *)malloc_d(maxLen + 1, "main:getPassword():pszPassword2");
	
	if (pszPassword1 == NULL) {
		throw new Exception(ERR_MALLOC, "Failed to allocate memory for password", __FILE__, "main", "getPassword", __LINE__);
	}
	
	cout << "Enter password: ";
	cout.flush();

	getpwd(pszPassword1, maxLen);
	
	cout << "Confirm password: ";
	cout.flush();

	getpwd(pszPassword2, maxLen);
	
	if (strcmp(pszPassword1, pszPassword2) != 0) {
		cout << "\nThe passwords do not match!" << endl;
		cout.flush();
		return -1;
	}
	
	strcpy_s(pszPassword, PASSWORD_BUFFER_LENGTH, pszPassword1);
	// strcpy_s(pszPassword, PASSWORD_BUFFER_LENGTH, "password");
	
	free_d(pszPassword1, "main:getPassword():pszPassword1");
	free_d(pszPassword2, "main:getPassword():pszPassword2");

	return 0;
}

/*
** Under windows, this uses getch(), under a different OS
** getch() probably won't be available, so you will have
** to use a different mechanism...
*/
void getpwd(char *pszPassword, int maxLen)
{
	int		ch;
	int		i;

#ifndef _WIN32
	struct termios current;
	struct termios original;

	tcgetattr(fileno(stdin), &original); /* grab old terminal i/o settings */
	current = original; /* make new settings same as old settings */
	current.c_lflag &= ~ICANON; /* disable buffered i/o */
	current.c_lflag &= ~ECHO; /* set echo mode */
	tcsetattr(fileno(stdin), TCSANOW, &current); /* use these new terminal i/o settings now */
#endif

	i = 0;
	ch = -1;
	
	while (ch != 0) {
	#ifdef _WIN32
		ch = _getch();
	#else
		ch = getchar();
	#endif
		
		if (ch != '\n' && ch != '\r') {
			if (i < maxLen) {
				cout << "*";
				cout.flush();
				pszPassword[i++] = (char)ch;
			}
			else {
				throw new Exception(ERR_INVALID_PWD_LEN, "The password is too long, must be < 64", __FILE__, "main", "getpwd", __LINE__);
			}
		}
		else {
			cout << endl;
			cout.flush();
			ch = 0;
			pszPassword[i++] = (char)ch;
		}
	}

#ifndef _WIN32
	tcsetattr(0, TCSANOW, &original);
#endif
}
