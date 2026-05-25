/*
**************
	ФИО: Агафонов Давид
	Вариант: 1
	(остальное палить на паблик репо не буду)
**************
*/

#include <cctype>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>

using namespace std;

const char *fileName = "planes.txt";

const int MAX_PLANES = 100;
const int LINE_SIZE = 256;
const int TIME_SIZE = 6;
const int MODEL_SIZE = 32;
const int BOARD_SIZE = 16;
const int CITY_SIZE = 32;

struct Plane
{
	char landingTime[TIME_SIZE];
	int landingMinutes;
	char model[MODEL_SIZE];
	char boardNumber[BOARD_SIZE];
	char departure[CITY_SIZE];
};

// Проверяет пустую строку; line - строка из файла.
bool isEmptyLine(const char line[]);

// Проверяет время; text - время, minutes - результат в минутах.
bool parseTime(const char text[], int &minutes);

// Проверяет борт; text - бортовой номер.
bool isGoodBoardNumber(const char text[]);

// Разбирает строку; line - строка, plane - запись, error - текст ошибки.
bool parsePlane(const char line[], Plane &plane, const char *&error);

// Проверяет борт и модель; planes - массив, count - размер, plane - новая запись.
bool checkPlaneLogic(const Plane planes[], int count, const Plane &plane, const char *&error);

// Читает файл; fileName - имя файла, planes - массив, count - число записей.
bool loadPlanes(const char fileName[], Plane planes[], int &count);

// Заполняет индексы; indexes - массив индексов, count - число записей.
void fillIndexes(int indexes[], int count);

// Сортирует индексы; planes - записи, indexes - индексы, count - число записей.
void sortIndexesByTime(const Plane planes[], int indexes[], int count);

// Печатает таблицу; planes - записи, indexes - порядок вывода, count - число записей.
void printTable(const Plane planes[], const int indexes[], int count);

int main(int argc, char *argv[])
{
	if (argc > 1)
	{
		fileName = argv[1];
	}

	Plane *planes = new Plane[MAX_PLANES];
	int *indexes = new int[MAX_PLANES];
	int count = 0;
	int result = 0;

	if (!loadPlanes(fileName, planes, count))
	{
		result = 1;
	}
	else
	{
		fillIndexes(indexes, count);
		sortIndexesByTime(planes, indexes, count);
		printTable(planes, indexes, count);
	}

	delete[] planes;
	delete[] indexes;

	return result;
}

bool isEmptyLine(const char line[])
{
	for (int i = 0; line[i] != '\0'; i++)
	{
		if (!isspace((unsigned char)line[i]))
		{
			return false;
		}
	}

	return true;
}

bool parseTime(const char text[], int &minutes)
{
	if (strlen(text) != 5 || text[2] != ':')
	{
		return false;
	}

	if (!isdigit((unsigned char)text[0]) || !isdigit((unsigned char)text[1]) ||
		!isdigit((unsigned char)text[3]) || !isdigit((unsigned char)text[4]))
	{
		return false;
	}

	int hours = (text[0] - '0') * 10 + (text[1] - '0');
	int mins = (text[3] - '0') * 10 + (text[4] - '0');

	if (hours > 23 || mins > 59)
	{
		return false;
	}

	minutes = hours * 60 + mins;
	return true;
}

bool isGoodBoardNumber(const char text[])
{
	// Буква "Б" в UTF-8 хранится двумя байтами: D0 91.
	unsigned char first = (unsigned char)text[0];
	unsigned char second = (unsigned char)text[1];

	if (first != 0xD0 || second != 0x91 || text[2] != '-')
	{
		return false;
	}

	for (int i = 3; i < 7; i++)
	{
		if (!isdigit((unsigned char)text[i]))
		{
			return false;
		}
	}

	return text[7] == '\0';
}

bool parsePlane(const char line[], Plane &plane, const char *&error)
{
	char time[TIME_SIZE];
	char model[MODEL_SIZE];
	char boardNumber[BOARD_SIZE];
	char departure[CITY_SIZE];
	char extra[LINE_SIZE];

	int fields = sscanf(line, "%5s %31s %15s %31s %255s",
						time, model, boardNumber, departure, extra);

	if (fields < 4)
	{
		error = "нужно 4 поля: время марка борт пункт";
		return false;
	}

	if (fields > 4)
	{
		error = "лишние данные в конце строки";
		return false;
	}

	int minutes = 0;
	if (!parseTime(time, minutes))
	{
		error = "время должно быть в формате HH:MM от 00:00 до 23:59";
		return false;
	}

	if (!isGoodBoardNumber(boardNumber))
	{
		error = "бортовой номер должен быть в формате Б-хххх";
		return false;
	}

	strcpy(plane.landingTime, time);
	plane.landingMinutes = minutes;
	strcpy(plane.model, model);
	strcpy(plane.boardNumber, boardNumber);
	strcpy(plane.departure, departure);

	return true;
}

bool checkPlaneLogic(const Plane planes[], int count, const Plane &plane, const char *&error)
{
	for (int i = 0; i < count; i++)
	{
		bool sameBoard = strcmp(planes[i].boardNumber, plane.boardNumber) == 0;
		bool sameTime = planes[i].landingMinutes == plane.landingMinutes;
		bool differentModel = strcmp(planes[i].model, plane.model) != 0;

		if (sameBoard && sameTime)
		{
			error = "один и тот же борт не может быть в одно и то же время";
			return false;
		}

		if (sameBoard && differentModel)
		{
			error = "один и тот же борт не может иметь разные модели";
			return false;
		}
	}

	return true;
}

bool loadPlanes(const char fileName[], Plane planes[], int &count)
{
	ifstream file(fileName);

	if (!file)
	{
		cout << "Ошибка: не удалось открыть файл " << fileName << "\n";
		return false;
	}

	count = 0;
	int lineNumber = 0;
	char line[LINE_SIZE];

	while (file.getline(line, LINE_SIZE))
	{
		lineNumber++;

		if (isEmptyLine(line))
		{
			cout << "Ошибка в строке " << lineNumber << ": пустая строка\n";
			return false;
		}

		if (count == MAX_PLANES)
		{
			cout << "Ошибка: записей больше " << MAX_PLANES << "\n";
			return false;
		}

		Plane plane;
		const char *error = "";

		if (!parsePlane(line, plane, error))
		{
			cout << "Ошибка в строке " << lineNumber << ": " << error << "\n";
			cout << "Строка: " << line << "\n";
			return false;
		}

		if (!checkPlaneLogic(planes, count, plane, error))
		{
			cout << "Ошибка в строке " << lineNumber << ": " << error << "\n";
			cout << "Строка: " << line << "\n";
			return false;
		}

		planes[count] = plane;
		count++;
	}

	if (file.fail() && !file.eof())
	{
		cout << "Ошибка: слишком длинная строка в файле\n";
		return false;
	}

	if (count == 0)
	{
		cout << "Ошибка: файл пустой\n";
		return false;
	}

	return true;
}

void fillIndexes(int indexes[], int count)
{
	for (int i = 0; i < count; i++)
	{
		indexes[i] = i;
	}
}

void sortIndexesByTime(const Plane planes[], int indexes[], int count)
{
	// Сортируем индексы. Сами записи самолетов не трогаем.
	for (int i = 0; i < count - 1; i++)
	{
		int minIndex = i;

		for (int j = i + 1; j < count; j++)
		{
			if (planes[indexes[j]].landingMinutes < planes[indexes[minIndex]].landingMinutes)
			{
				minIndex = j;
			}
		}

		int temp = indexes[i];
		indexes[i] = indexes[minIndex];
		indexes[minIndex] = temp;
	}
}

void printTable(const Plane planes[], const int indexes[], int count)
{
	cout << "\nСамолеты по возрастанию времени посадки:\n\n";
	cout << "Время | Марка        | Борт         | Пункт отправления\n";
	cout << "-------------------------------------------------------------\n";

	for (int i = 0; i < count; i++)
	{
		const Plane &plane = planes[indexes[i]];

		cout << left
			 << setw(5) << plane.landingTime << " | "
			 << setw(12) << plane.model << " | "
			 << setw(12) << plane.boardNumber << " | "
			 << plane.departure << "\n";
	}
}
