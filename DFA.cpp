#include <iostream>
#include <list>
#include <queue>
#include <tuple>
#include <stack>
#include <string>
#include <algorithm>

using Condition = int;
using Position = unsigned int;
using Symbol = char;


//описание класса автомата
class AutomatoWithStack {
private:
    //описание класса конкретной возможной ветви переходов автомата
    class ExactlyAutomatoBranching {
    public:
        Condition CurrentCondition;
        Position CurrentPosition;
        std::stack<char> Stack;

        ExactlyAutomatoBranching(const std::pair<Condition, std::string>& cond, const Position& current, const std::stack<char>& stack)
            : CurrentCondition(cond.first)
            , CurrentPosition(current)
            , Stack(stack) {
            if (!Stack.empty())
                Stack.pop();
            std::for_each(cond.second.rbegin(), cond.second.rend(), [this](char symb) {this->Stack.push(symb); });
            if (Stack.empty())
                Stack.push('\0');
        }
    };

    //Функция перехода НКА
    const std::list<std::pair<std::tuple<Condition, Symbol, char>, std::pair<Condition, std::string>>> SwitchingFunction;
    //Начальное состояние
    const Condition StartingCondition;
    //Множество конечных состояний
    const std::list<Condition> FinishingConditions;
    //Флаг, обозначающий, перешел ли какой-либо автомат в конечное состояние
    bool IsFinished = false;
    //Индекс, обозначающий последнюю позицию, куда автомат смог дойти
    int last_step = 0;
    //множество самоклонирующихся ветвей автомата
    std::queue<ExactlyAutomatoBranching> NonDeterminedAutomatosList;

    //берем произвольную ветвь
    //выполняем разветвления автомата,
    //если несколько возможных переходов
    auto Step(const std::string& CheckingString) -> void {
        if (NonDeterminedAutomatosList.empty())
            return;
        auto branching = NonDeterminedAutomatosList.front();
        NonDeterminedAutomatosList.pop();
        last_step = branching.CurrentPosition;
        auto ptr = std::find(FinishingConditions.begin(), FinishingConditions.end(), branching.CurrentCondition);
        if (ptr != FinishingConditions.end() && branching.CurrentPosition == CheckingString.size()) {
            IsFinished = true;
            return;
        }
        //сохранение состояния для перехода по считыванию символа
        auto StateTuple = std::tuple<Condition, Symbol, char>();
        if (branching.CurrentPosition != CheckingString.size()){
            StateTuple = std::tuple<Condition, Symbol, char>(branching.CurrentCondition
                , CheckingString[branching.CurrentPosition], char(branching.Stack.top()));
        }
        else {
            StateTuple = std::tuple<Condition, Symbol, char>(branching.CurrentCondition
                , '\b', char(branching.Stack.top()));
        }
        //сохранение состояния для Эпсилон перехода
        auto EpsilonStateTuple = std::make_tuple(branching.CurrentCondition
            , '\a', branching.Stack.top());
        //проход поиском по всей функции перехода
        std::for_each(SwitchingFunction.begin(), SwitchingFunction.end(),
            [this, &branching, &StateTuple, &EpsilonStateTuple](const std::pair<std::tuple<Condition, Symbol, char>, std::pair<Condition, std::string>>& ptr) {
                //поиск всех возможных переходов из текущего состояния автомата и копирование автомата
                if (ptr.first == StateTuple) {
                    auto copy = ExactlyAutomatoBranching(ptr.second, branching.CurrentPosition + 1, branching.Stack);
                    this->NonDeterminedAutomatosList.push(copy);
                }
                //поиск всех возможных E-переходов из текущего состояния автомата и копирование автомата
                else if (ptr.first == EpsilonStateTuple) {
                    auto copy = ExactlyAutomatoBranching(ptr.second, branching.CurrentPosition, branching.Stack);
                    this->NonDeterminedAutomatosList.push(copy);
                }

            });
    }
public:
    //конструктор класса
    AutomatoWithStack(
        std::list<std::pair<std::tuple<Condition, Symbol, char>, std::pair<Condition, std::string>>>  SwitchingFunction
        , const Condition& StartingCondition
        , std::list<Condition>  FinishingConditions
    ) :SwitchingFunction(std::move(SwitchingFunction))
        , StartingCondition(StartingCondition)
        , FinishingConditions(std::move(FinishingConditions)) {}

    //запуск проверки строки
    auto Check(const std::string& CheckingString) -> std::pair<bool, int> {
        //создание начального состояния автомата
        this->NonDeterminedAutomatosList.push(ExactlyAutomatoBranching(std::make_pair(StartingCondition, ""), 0, std::stack<char>()));
        //проверка всех возможных ветвлений
        while (!NonDeterminedAutomatosList.empty()) {
            Step(CheckingString);
            //если одно из ветвлений достигло результата - возвращаем истинность
            if (IsFinished)
                return std::make_pair(true, 0);
        }
        //если ни одно ветвление не достигло результата - возвращаем самую дальнюю точку, куда смог дойти автомат без ошибки
        return std::make_pair(false, last_step);
    }
};

int main() {
    //объявление функции перехода, где 
    //1)текущее состояние автомата
    //2)считываемый символ
    //3)символ на вершине стека
    //4)состояние, в которое перейдет автомат
    //5)символы, которые будут положены в стек, слева - останется на вершине
    auto switchingFunction = std::list<std::pair<std::tuple<Condition, Symbol, char>, std::pair<Condition, std::string>>>{
            std::make_pair(std::make_tuple(0, '0', '\0'), std::make_pair(1,"\0")),
            std::make_pair(std::make_tuple(0, '0', '0'), std::make_pair(1,"0")),
            std::make_pair(std::make_tuple(1, '0', '\0'), std::make_pair(0,"00\0")),
            std::make_pair(std::make_tuple(1, '0', '0'), std::make_pair(0,"000")),

            std::make_pair(std::make_tuple(0, '(', '\0'), std::make_pair(2,"\0")),
            std::make_pair(std::make_tuple(0, '(', '0'), std::make_pair(2,"0")),

            std::make_pair(std::make_tuple(2, '1', '\0'), std::make_pair(2,"1\0")),
            std::make_pair(std::make_tuple(2, '1', '0'), std::make_pair(2,"10")),
            std::make_pair(std::make_tuple(2, '1', '1'), std::make_pair(2,"11")),
            std::make_pair(std::make_tuple(2, '*', '\0'), std::make_pair(3,"\0")),
            std::make_pair(std::make_tuple(2, '*', '0'), std::make_pair(3,"0")),
            std::make_pair(std::make_tuple(2, '*', '1'), std::make_pair(3,"1")),

            std::make_pair(std::make_tuple(3, '1', '1'), std::make_pair(3,"")),
            std::make_pair(std::make_tuple(3, ')', '\0'), std::make_pair(4,"\0")),
            std::make_pair(std::make_tuple(3, ')', '0'), std::make_pair(4,"0")),

            std::make_pair(std::make_tuple(4, '0', '0'), std::make_pair(4,"")),
            std::make_pair(std::make_tuple(4, '\a', '\0'), std::make_pair(5,"")),
    };
    //объявление начального состояния автомата
    auto startingCondition = 0;
    //объявление множества конечных состояний
    auto finishingConditions = std::list<Condition>{ 5 };
    //создание автомата по требуемым параметрам
    AutomatoWithStack automato(switchingFunction, startingCondition, finishingConditions);

    //ввод строки, в которой будет осуществляться поиск
    auto str = std::string();
    std::cin >> str;

    //выполнение проверки
    auto records = automato.Check(str);
    //вывод результата на экран
    if (records.first) {
        std::cout << "String is correct";
    }
    else {
        std::cout << "String is incorrect" << std::endl;
        std::cout << "Mistake in position: " << (records.second + 1);
    }
    return 0;
}
