#ifndef TRMLSHELL_H
#define TRMLSHELL_H

#include "Lemmatizer.h"
#include "agramtab.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

using namespace std;
typedef map<QString, QString> multyformmap;

class TRMLShell
{
    //    LEMMATIZERLib::ILemmatizer *RusLemmatizer;
    //    AGRAMTABLib::IGramTab *RusGramTab;

    //        QAxObject *RusLemmatizer;
    //        QAxObject *RusGramTab;

    LEMMATIZERLib::LemmatizerRussian RusLemmatizer;
    AGRAMTABLib::RusGramTab RusGramTab;

    multyformmap mf;

public:
    TRMLShell();
    ~TRMLShell() {};

    QString AddNorms(
        QStringList *list,
        QString term,
        QString context,
        int &Interrupt,
        float fProgress,
        bool bFindNorms = true);
    QString GetNorm(
        QString term, QString context, int &Interrupt, float fProgress, bool bAskUser = false);

    unsigned char GetPOfS(QString term);
    void Save();
    bool CheckAbbr(QString str);
};

extern TRMLShell *lemmatizer;

#include <vector>
#include <QDateTime>

enum EObjectType {
    E_OBJECT_INVALID = 0,
    E_OBJECT_USER = 1,        // пользователь
    E_OBJECT_DISCIPLINE = 2,  // дисциплина
    E_OBJECT_FUND = 3,        // фонд
    E_OBJECT_SOURCE = 4,      // источник комплектования
    E_OBJECT_ALLUSERS = 5,    // все пользователи
    E_OBJECT_EXTERNALRES = 6, // внешний ресурс
    E_OBJECT_DOC = 7,         // документ
    E_OBJECT_SPECIALITY = 8,  // специализация
    E_OBJECT_COLLECTION = 9,  // коллекция документов
    E_OBJECT_DOCPART = 10     // часть документа
};

enum EAspect {
    E_ASPECT_INVALID = 0,
    E_ASPECT_ADD = 1,        // добавление
    E_ASPECT_DOWNLOAD = 2,   // скачивание
    E_ASPECT_ALLDOC = 3,     // все документы
    E_ASPECT_DISCDOC = 4,    // документы по дисциплине
    E_ASPECT_ALLRES = 5,     // все ресурсы
    E_ASPECT_REQUEST = 6,    // запросы
    E_ASPECT_SEARCHRES = 7,  // результаты поиска
    E_ASPECT_ALLUSERREQ = 8, // запросы всех пользователей
    E_ASPECT_DOCTEXT = 9,    // текст документа
    E_ASPECT_RESTEXT = 10,   // текст ресурса
    E_ASPECT_THEMEPLAN = 11, // тематический план по дисциплине
    E_ASPECT_ALLPLANS = 12, // тематические планы по всем дисциплинам
    E_ASPECT_ALLDISCDOCS = 13 // документы всех дисциплин
};

enum EAttribute {
    E_ATTR_INVALID = 0,
    E_ATTR_AUTHORS = 1,     // по авторам
    E_ATTR_DESCRIPTION = 2, // по описанию		// 2 (морфологический)
    E_ATTR_DISCIPLINE = 3,  // по дисциплине
    E_ATTR_PUBPLACE = 4,    // по месту издания
    E_ATTR_UDC = 5,         // по тематикам УДК // 3 (иерархический)
    E_ATTR_DOCTYPE = 6,     // по типу документа
    E_ATTR_SOURCE = 7,      // по источнику комплектования
    E_ATTR_PUBLISHER = 8,   // по издательству
    E_ATTR_PUBDATE = 9,     // по году издания
    E_ATTR_TITLE = 10,      // по заглавию
    E_ATTR_FULLTEXT = 11,   // полнотекстовый	// 2 (морфологический)
    E_ATTR_BBK = 12         // по тематикам ББК // 3 (иерархический)
};

enum EParamStructure {
    E_PARAMSTRUCT_INVALID = 0,
    E_PARAMSTRUCT_EXACT = 1,      // точный
    E_PARAMSTRUCT_MORPHOLOGY = 2, // морфологический
    E_PARAMSTRUCT_HIERARCHY = 3   // иерархический
};

//Класс описания параметров ИП
class IPPortraitDescription
{
public:
    ulong ulId; // Id сохраняемого ИП

    EObjectType eObjectType; // тип объекта
    EAspect eAspect;         // тип аспекта
    EAttribute eAttribute;   // тип параметра построения ИП
    EParamStructure
        eParamStruct; // структура параметра: точный, морфология, иерархия // NormalizationID
    vector<ulong> vObjectIds; // Массив идентификаторов объектов

    QDateTime dtReqAfter;  // Дата выборки запросов с...
    QDateTime dtReqBefore; // Дата выборки запросов по...

    ulong ulCreatorId; // ID пользователя составившего ИП
    QDateTime dtCreateTime; // int дата и время составления, редактирования ИП
    QString sComment; // Комментарий пользователя

    int iBackground; // Фон для создания портрета
    ulong ulPartOfDocId; // Принадлежность части док-та к самому документа
};

struct IPTerm
{
    ulong id;          // идентификатор
    QString term;      // термин
    double weight;     // вес
    double occurences; // частота в документе
    double dboccur;    // частота в БД
    ulong times;       // количество вхождений
    ulong dbtimes;     // количество вхождений в БД

    IPTerm() {};
    IPTerm(ulong i, QString t, double w, int tm) { id = i, term = t, weight = w, times = tm; }
};

struct IPShingle
{
    ulong id;          // идентификатор
    QString term;      // термин
    double weight;     // вес
    double occurences; // частота в документе
    double dboccur;    // частота в БД
    ulong shows;       // количество показов шингла
    ulong times;       // количество вхождений
    ulong dbtimes;     // количество вхождений в БД
    int size;          // размер шингла (количество слов)
    //    unsigned char   type;           // тип, 0 - термин, 1 - шингл

    IPShingle() {}
    IPShingle(ulong i, QString t, double w, int tm) { id = i, term = t, weight = w, times = tm; }
};

struct IPCompareResults
{
    double comm1; // вес общих терминов у ИП 1
    double comm2; // вес общих терминов у ИП 2
    double diff1; // вес различающихся терминов у ИП 1
    double diff2; // вес различающихся терминов у ИП 2

    double sum1; // суммарный вес ИП 1
    double sum2; // суммарный вес ИП 2

    double comm1prc; // процент (веса) общих терминов у ИП 1 (0..1)
    double comm2prc; // процент (веса) общих терминов у ИП 2
    double diff1prc; // процент (веса) различающихся терминов у ИП 1
    double diff2prc; // процент (веса) различающихся терминов у ИП 2

    double similiarity; // общая схожесть портетов (0..1)

    // круговая интерпретация
    double r1; // радиус круга ИП 1 (0..100)
    double r2; // радиус круга ИП 2
    double d;  // расстояние между центрами кругов

    // прямоугольная интерпретация
    double len1;     // длина ИП 1 (0..100)
    double len2;     // длина ИП 2
    double commlen1; // длина общих весов ИП 1
    double commlen2; // длина общих весов ИП 2

    double sim2; // общая схожесть портетов по терминам, другой вариант
    double simSh; // общая схожесть портетов по шинглам
    double simC;  // общая схожесть портетов
};

struct DocumentData
{
    int id;
    QString title;
    QString disciplines;
    QString themes;
    QString comment;
    QString mark;
    QString descHeader;
    QString descBody;
    QString authors;
    int typeDocId;
    QString udk;
    QString bbk;
    QString code;
    QString url;
    QString publishYear;
    QString publisher;
    QList<int> parts;
    int shingle_length;
    QString collection_id;

    QJsonObject toJson() const
    {
        QJsonObject jsonObj;
        jsonObj["id"] = id;
        jsonObj["title"] = title;
        jsonObj["disciplines"] = disciplines;
        jsonObj["themes"] = themes;
        jsonObj["comment"] = comment;
        jsonObj["mark"] = mark;
        jsonObj["desc_header"] = descHeader;
        jsonObj["desc_body"] = descBody;
        jsonObj["authors"] = authors;
        jsonObj["type_doc_id"] = typeDocId;
        jsonObj["udk"] = udk;
        jsonObj["bbk"] = bbk;
        jsonObj["code"] = code;
        jsonObj["url"] = url;
        jsonObj["publish_year"] = publishYear;
        jsonObj["publisher"] = publisher;

        QJsonObject indexParam;
        QJsonArray partsArray;
        for (int part : parts) {
            partsArray.append(part);
        }
        indexParam["parts"] = partsArray;
        indexParam["shingle_length"] = shingle_length;
        indexParam["collection-id"] = collection_id;

        jsonObj["index_param"] = indexParam;

        return jsonObj;
    }
};

struct IPExtendedCompareResults : IPCompareResults
{
    ulong ulWithId;     // id С КЕМ сравнивают
    ulong ulWhichId;    // id КОГО сравнивают
    QString withTitle;  // С КЕМ сравнивают
    QString whichTitle; // КОГО сравнивают

    bool operator<(const IPExtendedCompareResults &str) const { return !(sim2 < str.sim2); }
};

//Класс с данными ИП (сам портрет)
class IPPortrait : public IPPortraitDescription
{
public:
    ulong ulWordCounter;
    ulong ulShingleCounter;
    //	AnsiString		sSourceText;	// исходный текст
    QStringList slSourceText;
    //	std::map<char, std::vector<UnicodeString> > wordsCopies; // Копии оригинального текста для изменения в процессе поедания шинглов
    QStringList slAbbrs; // аббревиатуры
    //    Shingle *       pShingle;       // указатель на объект шингла

    IPPortrait();
    ~IPPortrait();

    map<QString, IPTerm> mData; // Данные ИП. Таблица с терминами и их весами
    map<QString, IPShingle> mSData; // Данные ИП. Таблица с шинглами и их весами

    // Функция создания ИП по параметрам описания
    /*	int Generate(ulong aOccurThreshold = 0, double aPercentThreshold = 0.,
                 double aWeightThreshold = 0.);
*/									// Функция создания ИП по данным внешнего ресурса
    virtual int GenerateByText(
        QString aText,
        bool bFindNorms = true,
        ulong aOccurThreshold = 0,
        double aPercentThreshold = 0.,
        double aWeightThreshold = 0.,
        QStringList *slNorms = 0);
    // Функция сравнения ИП
    /*
    void Compare(IPPortrait *ip2, IPCompareResults &res, vector<UnicodeString>* = NULL);
    IPExtendedCompareResults Compare(IPPortrait *ip2, vector<UnicodeString>* = NULL);
    vector<IPExtendedCompareResults>* IPPortrait::CompareMassively(vector<ulong> ipCompareList);
*/
    //--------------
    void CountWord(QString word);
    double GetWordOccurence(QString word);
    void CalcWordOccurences();

    /*
 * 	int GenTPByText(AnsiString aBeforeText, AnsiString aAfterText,
                            bool bFindNorms, TStringList *slNorms);
    void CountBeforeWord(AnsiString word, int index1, int total);
    void CountAfterWord(AnsiString word, int index1, int total);
*/
};

#endif // TRMLSHELL_H
