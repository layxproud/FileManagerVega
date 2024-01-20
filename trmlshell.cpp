#include "trmlshell.h"
#include <qaxobject.h>
#include <QFile>
#include <QMessageBox>
#include <QRegExp>

TRMLShell *lemmatizer = NULL;

TRMLShell::TRMLShell()
{
    RusLemmatizer.LoadDictionariesRegistry();
    RusGramTab.Load();

//    RusLemmatizer = new QAxObject("{6b23250f-1816-11d3-9cc6-00105a68baf2}");
//    RusLemmatizer->dynamicCall("LoadDictionariesRegistry()");
//    RusGramTab = new QAxObject("{ecd62f7e-19b6-11d3-a77a-2679a7000000}");
//    RusGramTab->dynamicCall("Load()");

    QFile DictFile("multiform.txt");

    if (DictFile.open(QIODevice::ReadOnly))
    {
        while(!DictFile.atEnd())
        {
            QStringList rec = QString(DictFile.readLine()).split(' ');
            mf[rec[0]] = rec[1];
        }

        DictFile.close();
    }
    else
        QMessageBox::information(0, "Ошибка открытия словаря multiform", DictFile.errorString());
}

QString TRMLShell::GetNorm(QString term, QString context, int &Interrupt, float fProgress, bool bAskUser)
{
    QString Lemma, SrcAncode;
    QStringList sl;
    long Founded = 0;
    unsigned char partofspeech = -1;
    unsigned long pid = 0;

    if (mf.count(term))
        return mf[term];

    LEMMATIZERLib::IParadigmCollection *ParadigmCollection = RusLemmatizer.CreateParadigmCollectionFromForm(term, 1, 1);

    if (!ParadigmCollection)
        return QString("");

    for (int k=0; k<ParadigmCollection->Count(); k++)
    {
        LEMMATIZERLib::IParadigm *item = ParadigmCollection->Item(k);
        pid = item->ParadigmID();
        Lemma = item->Norm();
        SrcAncode = item->SrcAncode();
        Founded = item->Founded();

        try
        {
            partofspeech = -1;
            if (Lemma.length() > 1)
                partofspeech = RusGramTab.GetPartOfSpeech(SrcAncode);
        }
        catch (...)
        {
            partofspeech = -1;
        }

        if ((partofspeech == 0 || partofspeech == 1) && Founded)
            break;

        Lemma = "";
    }

    return Lemma;

    //    QAxObject *ParadigmCollection = RusLemmatizer->querySubObject("CreateParadigmCollectionFromForm(const QString&, int, int)", term, 1, 1);
    //    int pcount = ParadigmCollection->dynamicCall("Count()").convert(QVariant::Int);
    //        QAxObject *item = ParadigmCollection->querySubObject("Item(int)", k);
    //        pid = item->property("ParadigmID").toInt();
    //        Lemma = item->property("Norm").toString();
    //        SrcAncode = item->property("SrcAncode").toString();
    //        Founded = item->property("Founded").toInt();
    //                partofspeech = RusGramTab->dynamicCall("GetPartOfSpeech(const QString&", SrcAncode).toUInt();
}

unsigned char TRMLShell::GetPOfS(QString term)
{
    QString Lemma, SrcAncode, RightLemma;
    long Founded = 0;
    unsigned char partofspeech = -1;
    unsigned long pid = 0;
    term = term.trimmed().toUpper();

    if (term.isEmpty())
        return -1;

    if (mf.count(term))
    {
        RightLemma = mf[term];
        if (RightLemma.isEmpty())
            RightLemma = " ";
    }
    else
        RightLemma = "";

    LEMMATIZERLib::IParadigmCollection *ParadigmCollection = RusLemmatizer.CreateParadigmCollectionFromForm(term, 1, 1);

    for (int k = 0; k < ParadigmCollection->Count(); ++k)
    {
        Lemma = ParadigmCollection->Item(k)->Norm();

        if (!RightLemma.isEmpty() && RightLemma != Lemma)
            continue;

        pid = ParadigmCollection->Item(k)->ParadigmID();
        SrcAncode = ParadigmCollection->Item(k)->SrcAncode();
        Founded = ParadigmCollection->Item(k)->Founded();

        try
        {
            partofspeech = -1;
            if (Lemma.length() > 1)
                partofspeech = RusGramTab.GetPartOfSpeech(SrcAncode);
        }
        catch (...)
        {
            partofspeech = -1;
        }

        if ((partofspeech == 0 || partofspeech == 1) && Founded)
            break;

        partofspeech = -1;
    }

    return partofspeech;
}

QString TRMLShell::AddNorms(QStringList *list, QString term, QString context, int &Interrupt, float fProgress, bool bFindNorms)
{
    if (term.trimmed().isEmpty())
        return "";

    QString norm;
    if (bFindNorms)
        norm = GetNorm(term, context, Interrupt, fProgress);
    else
        norm = term;
    if (!norm.isEmpty() && !Interrupt)
        list->append(norm);

    return norm;
}

void TRMLShell::Save()
{
    // not applicable
}

bool TRMLShell::CheckAbbr(QString str)
{
    QString uc = str.toUpper();

    if (str.trimmed().isEmpty() || (str != uc)
        || (str.trimmed().length() > 4)  || (str.trimmed().length() < 2))
        return false;

    LEMMATIZERLib::IParadigmCollection *ParadigmCollection = RusLemmatizer.CreateParadigmCollectionFromForm(str, 1, 1);

    QString Lemma, SrcAncode;
    long Founded = 0;
    unsigned char partofspeech = 0;
    unsigned long pid=0;

    for (int k=0; k<ParadigmCollection->Count(); k++)
    {
        pid = ParadigmCollection->Item(k)->ParadigmID();
        Lemma = ParadigmCollection->Item(k)->Norm();
        SrcAncode = ParadigmCollection->Item(k)->SrcAncode();
        Founded = ParadigmCollection->Item(k)->Founded();

        if (Founded)
            return false;
    }

    return true;
}


IPPortrait::IPPortrait()
{
    ulId = 0;
    ulCreatorId = 1;
    dtCreateTime = QDateTime::currentDateTime();

    ulWordCounter = 0;
    ulShingleCounter = 0;

    eObjectType = E_OBJECT_DOC;
    eAspect = E_ASPECT_DOCTEXT;
    eAttribute = E_ATTR_FULLTEXT;
    eParamStruct = E_PARAMSTRUCT_MORPHOLOGY;
}


IPPortrait::~IPPortrait()
{
}


void IPPortrait::CountWord(QString word)
{
    if (word.trimmed().isEmpty())
        return;

    ulWordCounter++;
    if (mData.count(word) == 0)
    {
        IPTerm term;
        term.term = word;
        term.times = 1;
        term.dboccur = 0.0;
        term.id = 0;
        term.weight = 0.0;
        term.occurences = 0.0;
        mData[word] = term;
    }
    else
        mData[word].times++;
}

void IPPortrait::CalcWordOccurences()
{
    map<QString,IPTerm>::iterator it = mData.begin();
    for ( ; it!=mData.end(); ++it)
    {
        it->second.occurences = (double)it->second.times / ulWordCounter;
    }
}

double IPPortrait::GetWordOccurence(QString word)
{
    if (word.trimmed().isEmpty())
        return 0.0;

    if (mData.count(word) > 0)
        return (mData[word].occurences = ((double)mData[word].times) / ulWordCounter);
    else
        return 0;
}


QStringList *SplitToWords(QString &sWholeText)
{
/*	QString str = sWholeText;
    for (int i=1; i<=str.Length(); i++)
    {
        if (str[i] == (char)0xAC)
        {
            str.Delete(i,1);
            i--;
            continue;
        }
        if (! ((str[i] >= 'a' && str[i] <= 'z') ||
                (str[i] >= 'A' && str[i] <= 'Z') ||
                (str[i] >= 'А' && str[i] <= 'Я') ||
                (str[i] >= 'а' && str[i] <= 'я'))
            )
            str[i]='\n';
    }

    TStringList *slWords = new TStringList;
    slWords->Text = str;*/
    QStringList *slWords = new QStringList(sWholeText.split(QRegExp("[^a-zA-Zа-яА-Я]")));
    return slWords;
}

int IPPortrait::GenerateByText(QString aText, bool bFindNorms, ulong aOccurThreshold,
                   double aPercentThreshold, double aWeightThreshold,
                   QStringList *slNorms)
{
    int Interrupt = 0;
    slSourceText = aText.split('\n');
    QStringList *slWords = SplitToWords(aText);

    QStringList *slForms = new QStringList;
    for(int i=0; i<slWords->count(); i++)
    {
        slForms->clear();
        if (!((*slWords)[i].trimmed().isEmpty()) && (slAbbrs.indexOf((*slWords)[i]) >= 0))
        {
            slForms->append(slWords[i]);
            if (slNorms)
                slNorms->append(slWords[i]);
        }
        else
        {
            QString context;
            if (slNorms)
                slNorms->append(lemmatizer->AddNorms(slForms, (*slWords)[i], context,
                    Interrupt, ((float)i)/slWords->count(), bFindNorms));
            else
                lemmatizer->AddNorms(slForms, (*slWords)[i], context, Interrupt,
                    ((float)i)/slWords->count(), bFindNorms);

            if (Interrupt)
            {
                lemmatizer->Save();
                CalcWordOccurences();
                return -1;
            }
        }
        for (int j=0; j<slForms->count(); j++)
        {
            if (!(*slForms)[j].trimmed().isEmpty())
                CountWord((*slForms)[j]);
        }
    }

    CalcWordOccurences();
    delete slForms;
    delete slWords;
    lemmatizer->Save();

    return 0;
}
