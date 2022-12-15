#include <iostream>
#include <pthread.h>
#include <vector>
#include <set>
#include <algorithm>
#include <unistd.h>

// структура, содержащая информацию, передаваемую потоку для работы.
// в переменной numbers находится список чисел, передаваемых потоку
// для изучения. в переменной result - результат работы, то есть все числа,
// удовлетворяющие условию, приведенному в задаче.
struct Data {
    std::vector<long long> numbers;
    std::vector<long long> result;
};

// параметр, задаваемый пользователем при начале работы с программой.
// нужен для проверки выполнения условия для чисел, в тексте задачи
// описывается, как "n".
int multiplier;
// параметр, задаваемый пользователем при начале работы с программой.
// нужен для формирования заданного количесвта потоков.
int threads_number;
// вспомогательная константа, максимальное число, до которого ведется изучение.
// примечание: для облегчения проверки корректности выполнения программы имеет
// смысл уменьшать число. Для больших чисел имеет место задержка на этапе
// распределения чисел между потоками.
const long long MAX_NUMBER = 100000000;
// мьютекс, предназначенный для отслеживания вывода данных, чтобы не 
// происходило конфликтов в КС при выводе.
pthread_mutex_t mutex;

// функция, находящая все значимые цифры переданного числа number. 
std::vector<int> getSignificantDigits(long long number) {
    std::vector<int> significantDigits; // массив значимых цифр.
    while (number != 0) { // цикл получения значимых цифр числа.
        int lastDigit = number % 10;
        significantDigits.push_back(lastDigit);
        number /= 10;
    }
    return significantDigits; // возврат массива со значимыми цифрами.
}

// функция, определяющая, выполняется ли для переданного числа 
// свойство, описанное в задаче. 
bool significantDigitsMatch(long long number, int multiplier) {
    std::vector<int> significantDigitsBefore = getSignificantDigits(number); // получение массива значимых цифр исходного числа.
    std::set<int> significantDigitsBeforeNoDoubles; // множество значимых цифр без повторений.
    for (auto i = significantDigitsBefore.begin(); i != significantDigitsBefore.end(); i++) {
        significantDigitsBeforeNoDoubles.insert(*i); 
    }
    size_t amountOfSignificantDigitsNoDoubles = significantDigitsBeforeNoDoubles.size();
    std::vector<int> significantDigitsAfter = getSignificantDigits(number * multiplier); // получение массива значимых цифр исходного числа, умноженного на n.
    for (auto i = significantDigitsAfter.begin(); i != significantDigitsAfter.end(); i++) {
        significantDigitsBeforeNoDoubles.insert(*i);
    }
    for (auto i = significantDigitsBeforeNoDoubles.begin(); i != significantDigitsBeforeNoDoubles.end(); i++) { // проверка, что все числа присутствуют в произведении.
        if (std::find(significantDigitsAfter.begin(), significantDigitsAfter.end(), *i) == significantDigitsAfter.end()) {
            return false;
        }
    }
    return amountOfSignificantDigitsNoDoubles == significantDigitsBeforeNoDoubles.size() && significantDigitsBefore.size() < 10; // проверка выполнения свойства.
}

// поточная функция для итеративного изучения переданных чисел на предмет выполнения указанного в задаче свойства.
void* getSuitableNumbers(void* numbers) {
    Data* data = (Data*)numbers; // переданная структура, хранящая числа и результат.
    for (auto i = 0; i < data->numbers.size(); i++) {
        if (significantDigitsMatch(data->numbers[i], multiplier)) { // в случае, если для числа выполняется свойство - добавляется в результат и выписывается.
            data->result.push_back(data->numbers[i]);
            pthread_mutex_lock(&mutex); // блокировка для отслеживания вывода.
            std::cout << "Found number: " << data->numbers[i] << ", after multiplication: " << data->numbers[i] * multiplier << "\n";
            pthread_mutex_unlock(&mutex); // разблокировка.
        }
    }
    return nullptr;
}

// вспомогательная функция для ввода числа n.
void getMultiplier() {
    std::cout << "Enter multiplier n: ";
    std::cin >> multiplier;
    while (multiplier < 2 || multiplier > 9) {
        std::cout << "Incorrect multiplier, should be between 2 and 9: ";
        std::cin >> multiplier;
    }
}

// вспомогательная функция для ввода числа потоков.
void getThreadsNumber() {
    std::cout << "Enter amount of threads: ";
    std::cin >> threads_number;
    while (threads_number < 1 || threads_number > 15) {
        std::cout << "Incorrect amount of threads, should be between 1 and 15: ";
        std::cin >> threads_number;
    }
}

// функция для получения результатов работы потоков.
void getResults(Data numbersExamined[]) {
    std::set<long long> result; // множество для хранения всех отобранных чисел.
    for (auto i = 0; i < threads_number; i++) {
        for (auto j = 0; j < numbersExamined[i].result.size(); j++) {
            result.insert(numbersExamined[i].result[j]);
        }
    }
    std::cout << "Found numbers: ";
    if (result.size() == 0) {
        std::cout << "no numbers";
    }
    for (auto i = result.begin(); i != result.end(); i++) {
        std::cout << *i << "; ";
    }
}

int main()
{
    getMultiplier(); // ввод данных
    getThreadsNumber();
    Data numbersExamined[threads_number]; // создание массива данных, которые будут переданы потокам.
    long long currentNumber = 1000; 
    std::cout << "Numbers are being distributed.\n";
    while (currentNumber < MAX_NUMBER) { // распределение чисел между потоками.
        for (long long i = 0; i < threads_number; i++) {
            numbersExamined[i].numbers.push_back(currentNumber + i);
        }
        currentNumber += threads_number;
    }
    sleep(2);
    pthread_t threads[threads_number]; // массив потоков.
    pthread_mutex_init(&mutex, nullptr); // инициализации мьютекса.
    for (auto i = 0 ; i < threads_number ; i++) { // создание потоков и передача им данных для работы.
        pthread_create(&threads[i], nullptr, getSuitableNumbers, (void*)&numbersExamined[i]) ;
    }
    for (int i = 0 ; i < threads_number; i++) { // ожидание завершения всех потоков.
        pthread_join(threads[i], nullptr);
    }
    getResults(numbersExamined); // вывод собранных результатов.
}