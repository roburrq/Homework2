#include<iostream>
#include<fstream>
#include<cstdlib>
#include<string>
#include<Windows.h>
using namespace std;

DWORD WINAPI THreadFun(LPVOID);
DWORD WINAPI THreadFun2(LPVOID);
DWORD WINAPI THreadFun3(LPVOID);
void connection();
void reading();
void writing();
void logDataInFile(string);

// handler
HANDLE hCreateFile;

// ReadFile Variables
BOOL fileReader;
DWORD numOfReadBytes;
char readBuffer[100];
DWORD readBufferSize = sizeof(readBuffer);

// WriteFile Variables
BOOL fileWriter;
DWORD numOfWrittenBytes;
char writeBuffer[100] = "ready";
DWORD writeBufferSize = sizeof(writeBuffer);

// check whether user gave connect command or not
bool isConnected = false;
// stop writing after sending stop to client
bool doWriting = false;

// string to store user input
string input;

int main()
{
	HANDLE readingThread = NULL;
	HANDLE writingThread = NULL;
	HANDLE keyBoardListener = NULL;
	DWORD rThreadId, wThreadId, kThreadId;

	// keyBoard thread
	keyBoardListener = CreateThread(
		NULL,
		0,
		THreadFun3,
		NULL,
		0,
		&kThreadId);

	if (keyBoardListener == NULL) {
		cout << "keyboard Thread creation error num : " << GetLastError() << endl;
		logDataInFile("keyboard Thread failed while creation");
	}

	while (true)
	{
		if (input == "exit")
		{
			// Close threads
			CloseHandle(readingThread);
			CloseHandle(writingThread);
			CloseHandle(keyBoardListener);
			// Close Pipe
			CloseHandle(hCreateFile);
			exit(0);
		}

		else if (input == "stop")
		{
			input = "";
			if (isConnected == false) {
				cout << "Error -> Connection is already closed...!!!" << endl;
				logDataInFile("Error -> Connection is already closed...!!!");
				continue;
			}
			else {
				isConnected = false;
				cout << "Stopping the flow" << endl;
			}
		}

		else if (input == "connect")
		{
			input = "";
			if (isConnected == true) {
				cout << "Error -> Connection is already established...!!!" << endl;
				logDataInFile("Error -> Connection is already established...!!!");
				continue;
			}

			connection();
			// if connection is not established
			if (hCreateFile == INVALID_HANDLE_VALUE) {
				cout << "Connection is not established because Server is close " << endl;
				logDataInFile("Connection is not established because Server is close ");
				continue;
			}

			isConnected = true;
			doWriting = true;

			// Writing thread
			writingThread = CreateThread(
				NULL,
				0,
				THreadFun2,
				NULL,
				0,
				&wThreadId);

			if (writingThread == NULL) {
				cout << "writingThread Thread creation error num : " << GetLastError() << endl;
				logDataInFile("WritingThread is failed while creation");
			}

			// reading thread
			readingThread = CreateThread(
				NULL,
				0,
				THreadFun,
				NULL,
				0,
				&rThreadId);

			if (readingThread == NULL) {
				cout << "ReadingThread creation error num : " << GetLastError() << endl;
				logDataInFile("ReadingThread is failed while creation");
			}
		}
	}
	system("PAUSE");
	return 0;
}

DWORD WINAPI THreadFun3(LPVOID lpParam)
{
	while (true) {
		getline(cin, input);
	}
	return 0;
}

DWORD WINAPI THreadFun(LPVOID lpParam)
{
	while (isConnected) {
		reading();
	}
	return 0;
}

DWORD WINAPI THreadFun2(LPVOID lpParam)
{
	while (doWriting) {
		writing();
	}
	return 0;
}

void connection()
{
	// CreateFile for Pipe
	hCreateFile = CreateFile(
		L"\\\\.\\pipe\\ICS0025",
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
}

void reading()
{
	// ReadFile
	fileReader = ReadFile(
		hCreateFile,
		readBuffer,
		readBufferSize,
		&numOfReadBytes,
		NULL);
	if (fileReader == FALSE)
	{
		cout << "ReadFile failed with error number: " << GetLastError() << endl;
		logDataInFile("ReadFile failed");
	}
	else
		logDataInFile("DATA READING FROM SERVER -> " + (string)readBuffer);

	// Print the Server response
	cout << "DATA READING FROM SERVER -> " << readBuffer << endl;
}

void writing()
{
	// WriteFile
	if (isConnected == true)
	{
		fileWriter = WriteFile(
			hCreateFile,
			writeBuffer,
			writeBufferSize,
			&numOfWrittenBytes,
			NULL);
		if (fileWriter == FALSE) {
			cout << writeBuffer << " is Written failed with error number: " << GetLastError() << endl;
			logDataInFile("Ready fails to Write on pipe");
		}
		else
			cout << writeBuffer << " is Written successfully" << endl;
	}
	else if (doWriting == true)
	{
		char writeStop[100] = "stop";

		fileWriter = WriteFile(
			hCreateFile,
			writeStop,
			writeBufferSize,
			&numOfWrittenBytes,
			NULL);
		if (fileWriter == FALSE) {
			cout << writeStop << " is Written failed with error number: " << GetLastError() << endl;
			logDataInFile("Stop fails to Write on pipe");
		}
		else
			cout << writeStop << " is Written successfully" << endl;
		// to stop writing any furher message
		doWriting = false;
	}
}

void logDataInFile(string data)
{
	ofstream fout;
	ifstream fin;
	fin.open("history.txt");

	//To open file in Append mode
	fout.open("history.txt", ios::app);

	if (fin.is_open())
		fout << data << endl;

	fin.close();
	fout.close();
}