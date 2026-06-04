/******************************************************************************
 *                      КАФЕДРА №304 1 КУРС ПРОГИНЖ                            *
 *                           Летняя Практика                                   *
 *-----------------------------------------------------------------------------*
 * Project Type  : macOS Console Application                                   *
 * Project Name  : mai_practice                                                *
 * File Name     : main.cpp                                                    *
 * Language      : C/C++                                                       *
 * Programmer    : Агафонов Давид                                              *
 * Modified By   :                                                             *
 * Created       : 26/05/2026                                                  *
 * Comment(s)    : Работа со структурами и индексной сортировкой               *
 ******************************************************************************/

#include <cctype>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>

using namespace std;

const char DEFAULT_FILE[] = "planes.txt";			// файл по умолчанию
const char POSITIVE_FILE[] = "planes_positive.txt"; // хорошие данные
const char NEGATIVE_FILE[] = "planes_negative.txt"; // плохие данные
const char MIXED_FILE[] = "planes_mixed.txt";		// хорошие и плохие данные
const char BOUNDARY_FILE[] = "planes_boundary.txt"; // проверки границ

const int MAX_PLANES = 100; // максимум записей для динамического массива
const int MAX_ERRORS = 100;
const int ERROR_SIZE = 320;
const int LINE_SIZE = 256;
const int TIME_SIZE = 6;
const int MODEL_SIZE = 32;
const int BOARD_SIZE = 16;
const int CITY_SIZE = 64;

struct Plane
{
	char landingTime[TIME_SIZE];
	int landingMinutes;
	char model[MODEL_SIZE];
	char boardNumber[BOARD_SIZE];
	char city[CITY_SIZE];
};

/****************************************************************/
/*          П Р О Т О Т И П Ы     Ф У Н К Ц И Й                 */
/****************************************************************/

// проверяет пустую строку, line - строка из файла
bool isEmptyLine(const char line[]);

// проверяет цифру, ch - один символ
bool isDigit(char ch);

// проверяет время, text - время, minutes - результат в минутах
bool parseTime(const char text[], int &minutes);

// проверяет борт, text - бортовой номер
bool isGoodBoardNumber(const char text[]);

// разбирает строку, line - строка, plane - запись, error - текст ошибки
bool parsePlane(const char line[], Plane &plane, const char *&error);

// проверяет борт и модель, planes - массив, count - размер, plane - новая запись
bool checkPlaneLogic(const Plane planes[], int count, const Plane &plane, const char *&error);

// добавляет ошибку, errors - список, lineNumber - номер строки
void addError(char errors[][ERROR_SIZE], int &errorCount, int lineNumber, const char line[], const char message[]);

// читает файл, fileName - имя, planes - записи, errors - список ошибок
bool loadPlanes(const char fileName[], Plane planes[], int &count, char errors[][ERROR_SIZE], int &errorCount);

// сортирует индексы, planes - записи, indexes - индексы, count - число записей
void sortIndexesByTime(const Plane planes[], int indexes[], int count);

// печатает таблицу, planes - записи, indexes - порядок вывода, count - число записей
void printTable(const Plane planes[], const int indexes[], int count);

/****************************************************************/
/*             О С Н О В Н А Я     П Р О Г Р А М М А            */
/****************************************************************/

int main(int argc, char *argv[])
{
	const char *fileName = DEFAULT_FILE;
	if (argc > 1)
	{
		fileName = argv[1];
	}

	Plane *planes = new Plane[MAX_PLANES];
	int *indexes = new int[MAX_PLANES];
	char errors[MAX_ERRORS][ERROR_SIZE];
	int count = 0;
	int errorCount = 0;
	int result = 0;

	if (!loadPlanes(fileName, planes, count, errors, errorCount))
	{
		result = 1;
	}

	if (errorCount > 0)
	{
		cout << "\nОшибки:\n";
		for (int i = 0; i < errorCount; i++)
		{
			cout << "- " << errors[i] << "\n";
		}
	}

	if (count == 0)
	{
		cout << "\nКорректных записей для таблицы нет\n";
	}
	else
	{
		// заполняем индексы перед сортировкой
		for (int i = 0; i < count; i++)
		{
			indexes[i] = i;
		}

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

bool isDigit(char ch)
{
	return ch >= '0' && ch <= '9';
}

bool parseTime(const char text[], int &minutes)
{
	if (strlen(text) != 5 || text[2] != ':')
	{
		return false;
	}

	if (!isDigit(text[0]) || !isDigit(text[1]) ||
		!isDigit(text[3]) || !isDigit(text[4]))
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
	// буква Б в UTF-8 хранится двумя байтами
	unsigned char first = (unsigned char)text[0];
	unsigned char second = (unsigned char)text[1];

	if (first != 0xD0 || second != 0x91 || text[2] != '-')
	{
		return false;
	}

	for (int i = 3; i < 7; i++)
	{
		if (!isDigit(text[i]))
		{
			return false;
		}
	}

	return text[7] == '\0';
}

bool parsePlane(const char line[], Plane &plane, const char *&error)
{
	int position = 0;

	int fields = sscanf(line, "%5s %31s %15s %n", plane.landingTime,
						plane.model, plane.boardNumber, &position);

	if (fields < 3)
	{
		error = "нужно минимум 4 поля: время марка борт пункт";
		return false;
	}

	while (line[position] == ' ' || line[position] == '\t')
	{
		position++;
	}

	if (line[position] == '\0')
	{
		error = "отсутствует пункт отправления";
		return false;
	}

	if (strlen(line + position) >= CITY_SIZE)
	{
		error = "пункт отправления слишком длинный";
		return false;
	}

	int minutes = 0;
	if (!parseTime(plane.landingTime, minutes))
	{
		error = "время должно быть в формате HH:MM от 00:00 до 23:59";
		return false;
	}

	if (!isGoodBoardNumber(plane.boardNumber))
	{
		error = "бортовой номер должен быть в формате Б-хххх";
		return false;
	}

	plane.landingMinutes = minutes;
	strcpy(plane.city, line + position);

	return true;
}

void addError(char errors[][ERROR_SIZE], int &errorCount, int lineNumber, const char line[], const char message[])
{
	if (errorCount >= MAX_ERRORS)
	{
		return;
	}

	if (lineNumber > 0)
	{
		snprintf(errors[errorCount], ERROR_SIZE, "Строка %d: %s | %s", lineNumber, message, line);
	}
	else
	{
		snprintf(errors[errorCount], ERROR_SIZE, "%s", message);
	}

	errorCount++;
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

bool loadPlanes(const char fileName[], Plane planes[], int &count, char errors[][ERROR_SIZE], int &errorCount)
{
	ifstream file(fileName);

	if (!file)
	{
		char message[ERROR_SIZE];
		snprintf(message, ERROR_SIZE, "не удалось открыть файл %s", fileName);
		addError(errors, errorCount, 0, "", message);
		cout << "Файлы для проверки: " << DEFAULT_FILE << ", " << POSITIVE_FILE << ", "
			 << NEGATIVE_FILE << ", " << MIXED_FILE << ", " << BOUNDARY_FILE << "\n";
		return false;
	}

	count = 0;
	int lineNumber = 0;
	char line[LINE_SIZE];

	while (true)
	{
		file.getline(line, LINE_SIZE);

		if (!file)
		{
			if (file.eof())
			{
				break;
			}

			if (file.fail())
			{
				lineNumber++;
				addError(errors, errorCount, lineNumber, "", "слишком длинная строка");
				file.clear();
				file.ignore(10000, '\n');
				continue;
			}
		}

		lineNumber++;

		if (isEmptyLine(line))
		{
			addError(errors, errorCount, lineNumber, line, "пустая строка");
			continue;
		}

		if (count == MAX_PLANES)
		{
			addError(errors, errorCount, lineNumber, line, "слишком много корректных записей");
			continue;
		}

		Plane plane;
		const char *error = "";

		if (!parsePlane(line, plane, error))
		{
			addError(errors, errorCount, lineNumber, line, error);
			continue;
		}

		if (!checkPlaneLogic(planes, count, plane, error))
		{
			addError(errors, errorCount, lineNumber, line, error);
			continue;
		}

		planes[count] = plane;
		count++;
	}

	if (lineNumber == 0)
	{
		addError(errors, errorCount, 0, "", "файл пустой");
	}
	else if (count == 0)
	{
		addError(errors, errorCount, 0, "", "корректных записей в файле нет");
	}

	return true;
}

void sortIndexesByTime(const Plane planes[], int indexes[], int count)
{
	// сортируем индексы, сами записи самолетов не трогаем
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
	cout << "Время | Марка | Борт | Пункт отправления\n";
	cout << "-----------------------------------------\n";

	for (int i = 0; i < count; i++)
	{
		const Plane &plane = planes[indexes[i]];

		cout << plane.landingTime << " | "
			 << plane.model << " | "
			 << plane.boardNumber << " | "
			 << plane.city << "\n";
	}
}

/* -------------    КОНЕЦ ФАЙЛА main.cpp    ------------- */
