/* C++ 17 file
 * Date 20201124
 * Author Liao
 * Individual std::map<unsigned int, std::map<std::wstring, double>>
 * Center std::map<unsigned int, std::map<std::wstring, double>>
 * kmeans algorithm
 * parameter I/O default detail
 */
#include "iostream"
#include "map"
#include "list"
// SQL Tool
#include "sqltools"
// CSV
#include "csvop"

// Login ID
const db_info login("DSSW", "sa", "123456");

// ��ȡ����Դ
/* Database
 * DBLogID      input   -       Database login information
 * strTableName input   -       Database table name
 * wstrFKStr    input   L"id"   Individual primary key
 */
std::map<int64_t, std::map<std::wstring, double>> GetDataSrc(db_info DBLogID, std::string strTableName, std::wstring wstrFKStr = L"id")
{
    std::map<int64_t, std::map<std::wstring, double>> mapDataSrc;
    if(!DBLogID.IsEmpty())
    {
        // Get database's data
        auto rsDataSrc = sql_server::go(DBLogID, "select * from " + strTableName);
        // Resultset iteration
        for(int64_t i=0; i<rsDataSrc.get_row_count(); i++)
        {
            std::map<std::wstring, double> mapDataElem;
            // Individual cell's members
            for(auto temp : rsDataSrc.get_result(i)) if(temp.first != wstrFKStr)
                mapDataElem.insert(std::pair<std::wstring, double>(temp.first, stod(temp.second)));
            mapDataSrc.insert(std::pair<int64_t, std::map<std::wstring, double>>(stod(rsDataSrc.get_result(i)[wstrFKStr]), mapDataElem));
        }
    }
    return mapDataSrc;
}
/* CSV file -overload-
 * strFileDir   input   -       CSV file's directory
 * bDataNoCol   input   false   Whether a serial number column exist
 * bDataFdNoLn  input   false   Whether a field line exist
 */
std::map<int64_t, std::map<std::wstring, double>> GetDataSrc(std::string strFileDir, bool bDataNoCol = false, bool bDataFdNoLn = false)
{
    std::map<int64_t, std::map<std::wstring, double>> mapDataSrc;
    // CSV header
    std::map<uint64_t, std::wstring> mapFdName;
    if(strFileDir.length())
    {
        /* Get CSV table
         * std::list<std::list<string>> lsTab
         */
        auto lsTab = input_table(strFileDir);
        auto iterTabLn = lsTab.begin();
        // Get table's header map
        if(bDataFdNoLn)
        {
            auto iterTabHead = iterTabLn->begin();
            if(bDataNoCol) iterTabHead ++;
            auto nHeadCnt = 0;
            while(iterTabHead != iterTabLn->end())
            {
                // <SerialNumber, HeaderName>
                mapFdName.insert(std::pair<uint64_t, std::wstring>(nHeadCnt, charset_exchange(*iterTabHead)));
                nHeadCnt ++;
                iterTabHead ++;
            }
            iterTabLn ++;
        }
        // Get table's data
        auto nLnCnt = 0;
        while(iterTabLn != lsTab.end())
        {
            std::map<std::wstring, double> mapDataElem;
            auto nDataNo = 0;
            auto iterTabCol = iterTabLn->begin();
            if(bDataNoCol)
            {
                nDataNo = stoi(*iterTabCol);
                iterTabCol ++;
            }
            else nDataNo = nLnCnt;
            auto nColCnt = 0;
            // Element's members
            while(iterTabCol != iterTabLn->end())
            {
                std::wstring wstrFdName = L"";
                if(bDataNoCol) wstrFdName = mapFdName[nColCnt];
                else wstrFdName = to_wstring(nColCnt);
                mapDataElem.insert(std::pair<std::wstring, double>(wstrFdName, stod(*iterTabCol)));
                nColCnt ++;
                iterTabCol ++;
            }
            mapDataSrc.insert(std::pair<int64_t, std::map<std::wstring, double>>(nDataNo, mapDataElem));
            iterTabLn ++;
            nLnCnt ++;
        }
    }
    return mapDataSrc;
}
// ��ȡ��һ������
/* Get next cluster iteration center
 * lsCltr   input   -   Previous cluster value
 */
std::map<int64_t, std::map<std::wstring, double>> GetNextCenter(std::map<int64_t, std::map<int64_t, std::map<std::wstring, double>>> lsCltr)
{
    std::map<int64_t, std::map<std::wstring, double>> mapNewCenter;
    // ��ʼ������
    for(auto mapCltrTemp : lsCltr)
    {
        std::map<std::wstring, double> mapNewCentElem;
        mapNewCenter.insert(std::pair<int64_t, std::map<std::wstring, double>>(mapCltrTemp.first, mapNewCentElem));
    }
    for(auto mapCltrTemp : lsCltr)
    {
        std::map<std::wstring, double> mapSum;
        for(auto mapCltrElemTemp : mapCltrTemp.second) for(auto mapElemTemp : mapCltrElemTemp.second)
        if(mapSum.size() == mapCltrElemTemp.second.size()) mapSum[mapElemTemp.first] += mapElemTemp.second;
        else mapSum.insert(mapElemTemp);
        for(auto mapSumTemp : mapSum) mapSum[mapSumTemp.first] /= mapCltrTemp.second.size();
        mapNewCenter[mapCltrTemp.first] = mapSum;
    }
    return mapNewCenter;
}

// ŷʽ�����ƽ��
/* Get Euclid distance's square value
 * mapFirst/Second  input - Two cluster's element
 */
double GetEuclidDistPow(std::map<std::wstring, double> mapFirst, std::map<std::wstring, double> mapSecond)
{
    double dESum = 0;
    for(auto mapTemp : mapFirst) dESum += pow(mapTemp.second-mapSecond[mapTemp.first], 2);
    return dESum;
}

// ׼����
/* Get standard eigen function value
 * lsCltr   input   -   Current iteration cluster
 */
double GetStdEVal(std::map<int64_t, std::map<int64_t, std::map<std::wstring, double>>> lsCltr)
{
    // ��ȡ��һ������
    auto mapNextCent = GetNextCenter(lsCltr);
    double dStdEVal = 0;
    // ʹ��ŷ�Ͼ��������������ֵ
    for(int64_t i=0; i<lsCltr.size(); i++)
    {
        double dStadEValElem = 0;
        for(auto lsTemp : lsCltr[i])
            dStadEValElem += GetEuclidDistPow(lsTemp.second, mapNextCent[i]);
        dStdEVal += dStadEValElem;
    }
    return dStdEVal;
}

// ��ȡ��һ������
/* Get next iteration
 * lsCltr   input   -   Current iteration cluster
 */
std::map<int64_t, std::map<int64_t, std::map<std::wstring, double>>> GetNextCluster(std::map<int64_t, std::map<int64_t, std::map<std::wstring, double>>> lsCltr)
{
    auto mapNextCent = GetNextCenter(lsCltr);
    std::map<int64_t, std::map<int64_t, std::map<std::wstring, double>>> mapNextCltr;
    // ��ʼ��������
    for(auto mapNewCentTemp : mapNextCent)
    {
        std::map<int64_t, std::map<std::wstring, double>> lsSgcltr;
        mapNextCltr.insert(std::pair<int64_t, std::map<int64_t, std::map<std::wstring, double>>>(mapNewCentTemp.first, lsSgcltr));
    }
    for(auto mapTemp : lsCltr) for(auto lsTemp : mapTemp.second)
    {
        auto nCtrlNo = mapNextCent.begin()->first;
        // ����ŷ�Ͼ���
        auto dMinEDist = GetEuclidDistPow(lsTemp.second, mapNextCent.begin()->second);
        for(auto mapElemTemp : mapNextCent)
        {
            auto dCurMinDist = GetEuclidDistPow(lsTemp.second, mapElemTemp.second);
            // ���վ���ֵ��������������
            if(dCurMinDist < dMinEDist)
            {
                dMinEDist = dCurMinDist;
                nCtrlNo = mapElemTemp.first;
            }
        }
        mapNextCltr[nCtrlNo].insert(lsTemp);
    }
    return mapNextCltr;
}

// K-Means ����
/* Core function
 * mapDataSrc   input   -       Data source
 * mapCltrCent  refer   -       Initial cluster center
 * nCltrCentNum input   3       Amount of cluster
 * accuracy     input   1e-5    Calculation accuracy
 */
std::map<int64_t, std::map<int64_t, std::map<std::wstring, double>>> GetCluster(std::map<int64_t, std::map<std::wstring, double>> mapDataSrc, std::map<int64_t, std::map<std::wstring, double>> &mapCltrCent, int64_t nCltrCentNum = 3, double accuracy = 1e-5)
{
    std::map<int64_t, std::map<int64_t, std::map<std::wstring, double>>> mapCltr;
    for(int64_t i=0; i<nCltrCentNum; i++)
    {
        std::map<int64_t, std::map<std::wstring, double>> mapCltrElem;
        mapCltr.insert(std::pair<int64_t, std::map<int64_t, std::map<std::wstring, double>>>(i, mapCltrElem));
    }
    // ��ʼ������
    int64_t nInitCnt = 0;
    for(auto mapDataTemp : mapDataSrc)
    {
        mapCltr[nInitCnt].insert(mapDataTemp);
        if(nInitCnt + 1 != nCltrCentNum) nInitCnt ++;
    }
    // ��ʼ��׼�����
    auto dPriorEVal = GetStdEVal(mapCltr);
    bool bIter = true;
    do
    {
        mapCltrCent = GetNextCenter(mapCltr);
        // ��һ������
        mapCltr = GetNextCluster(mapCltr);
        auto dNextEVal = GetStdEVal(mapCltr);
        if(abs(dNextEVal-dPriorEVal) < accuracy) bIter = false;
        dPriorEVal = dNextEVal;
    }
    while(bIter);
    return mapCltr;
}

/* Get 2D discrete graph position
 * lsCltr       input   -   Current cluster
 * lsCltrCent   input   -   Current cluster center
 */
std::map<int64_t, std::pair<double, double>> GetClusterGraphLoc(std::map<int64_t, std::map<int64_t, std::map<std::wstring, double>>> lsCltr, std::map<int64_t, std::map<std::wstring, double>> lsCltrCent)
{
    std::map<int64_t, std::pair<double, double>> mapLoc;
    for(int64_t i=0; i<lsCltr.size(); i++) for(auto mapTemp : lsCltr[i])
    {
        auto dEucY = GetEuclidDistPow(lsCltrCent[i], mapTemp.second);
        auto dEucX = pow(dEucY, 0.5);
        mapLoc.insert(std::pair<int64_t, std::pair<double, double>>(mapTemp.first, std::pair<double, double>(dEucX, dEucY)));
    }
    return mapLoc;
}


int main(int argc, char **argv, char **envp)
{
    std::cout << "Hello World!" << std::endl;
    std::map<int64_t, std::map<std::wstring, double>> mapCenter;
    auto mapCluster = GetCluster(GetDataSrc("ex2.csv", true, true), mapCenter, 3, 1e-5);
    return EXIT_SUCCESS;
}