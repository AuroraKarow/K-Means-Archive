/* C++ 17 file
 * Date 20201124
 * Author Liao
 */

#include "sqltools"
#include "matrix"

// 正负界收敛操作
void MatFreshZero(matrix &mat_var)
{
    for(UINT i=0; i<mat_var.get_line(); i++)
        for(UINT j=0; j<mat_var.get_column(); j++)
            if(mat_var[i][j] == -0) mat_var[i][j] = 0;
}

// 单纯形法是否有可用解
bool IsItrOptm(matrix vecX)
{
    for(UINT i=0; i<vecX.get_column(); i++)
    {
        bool bIsItr = false;
        for( UINT j=0; j<vecX.get_line(); j++)
            if(vecX[j][i] >= 0)
            {
                bIsItr = true;
                break;
            }
        if(!bIsItr) return false;
    }
    return true;
}

// 判定单纯形法是否迭代完成
bool IsComplete(matrix vecMain)
{
    for(UINT i=0; i<vecMain.get_column(); i++) if(vecMain[0][i] < 0) return false;
    return true;
}

// 单纯形法获取进基项
UINT GetMinBaseLnPos(matrix vecB, matrix vecX, UINT nMainCol)
{
    std::map<UINT, double> mapRate;
    for(UINT i=0; i<vecX.get_line(); i++) if(vecX[i][nMainCol] > 0)
        mapRate.insert(std::pair<UINT, double>(i, vecB[i][0]/vecX[i][nMainCol]));
    auto nMinPos = mapRate.begin()->first;
    for(auto temp : mapRate) if(temp.second <= mapRate[nMinPos]) nMinPos = temp.first;
    return nMinPos;
}

// 单纯形法矩阵行变换
void MatColUnitProc(matrix &vecMain, matrix &vecB, matrix &vecX, UINT nLnPos, UINT nColPos)
{
    auto dProVar = vecX[nLnPos][nColPos];
    for(UINT i=0; i<vecX.get_column(); i++) vecX[nLnPos][i] /= dProVar;
    vecB[nLnPos][0] /= dProVar;
    for(UINT i=0; i<vecX.get_line(); i++) if(i != nLnPos && vecX[i][nColPos])
    {
        auto dCurAdp = vecX[i][nColPos];
        for(UINT j=0; j<vecX.get_column(); j++)
        {
            auto dProElem = vecX[nLnPos][j] * abs(dCurAdp);
            if(dCurAdp > 0) vecX[i][j] -= dProElem;
            else vecX[i][j] = vecX[i][j] += dProElem;
        }
        auto dProB = vecB[nLnPos][0] * abs(dCurAdp);
        if(dCurAdp > 0) vecB[i][0] -= dProB;
        else vecB[i][0] += dProB;
    }
    auto dCurMain = vecMain[0][nColPos];
    for(UINT j=0; j<vecMain.get_column(); j++)
    {
        auto dProMain = vecX[nLnPos][j] * abs(dCurMain);
        if(dCurMain > 0) vecMain[0][j] -= dProMain;
        else vecMain[0][j] += dProMain;
    }
}

// 单纯形法
bool StdSimpAlg(matrix &vecMain, matrix &vecB, matrix &vecX)
{
    while(!IsComplete(vecMain)) if(IsItrOptm(vecX))
    {
        auto nMainPosCol = vecMain.min_value(0, true);
        auto nMainPosLn = GetMinBaseLnPos(vecB, vecX, nMainPosCol);
        MatColUnitProc(vecMain, vecB, vecX, nMainPosLn, nMainPosCol);
        MatFreshZero(vecMain);
        MatFreshZero(vecB);
        MatFreshZero(vecX);
    }
    else return false;
    return true;
}

// 根据系数矩阵与增广矩阵获取特解
matrix GetSipAlgResult(matrix vecB, matrix vecX, bool bInt)
{
    matrix matRes(vecX.get_column(), 1);
    for(UINT i=0; i<vecX.get_column(); i++)
    {
        bool bIsRes = true;
        UINT nUnitNo = 0, nUnitCnt = 0;
        double dRes = 0;
        for(UINT j=0; j<vecX.get_line(); j++)
        {
            if((vecX[j][i] != 1 && vecX[j][i] != 0))
            {
                matRes[i][0] = 0;
                bIsRes = false;
                break;
            }
            if(vecX[j][i] == 1)
            {
                nUnitCnt ++;
                nUnitNo = j;
                if(bInt) dRes = round(vecB[j][0]);
                else dRes = vecB[j][0];
            }
        }
        if(bIsRes && nUnitCnt==1) matRes[i][0] = dRes;
        else matRes[i][0] = 0;
    }
    return matRes;
}

int main(int argc, char **argv, char **envp)
{
    std::cout << "Hello World!" << std::endl;

    matrix vecB(5, 1);
    matrix vecX(5, 8);
    matrix vecMain(1, 8);

    // 赋值
    vecMain[0][0] = -192; vecMain[0][1] = -36; vecMain[0][2] = -12; vecMain[0][3] = 0;
    vecMain[0][4] = 0; vecMain[0][5] = 0; vecMain[0][6] = 0; vecMain[0][7] = 0;

    vecX[0][0] = 150000; vecX[0][1] = 0; vecX[0][2] = 0; vecX[0][3] = -1;
    vecX[0][4] = 0; vecX[0][5] = 0; vecX[0][6] = 0; vecX[0][7] = 0;
    vecX[1][0] = 1; vecX[1][1] = 0; vecX[1][2] = 0; vecX[1][3] = 0;
    vecX[1][4] = -1; vecX[1][5] = 0; vecX[1][6] = 0; vecX[1][7] = 0;
    vecX[2][0] = 0; vecX[2][1] = 1; vecX[2][2] = 0; vecX[2][3] = 0;
    vecX[2][4] = 0; vecX[2][5] = -1; vecX[2][6] = 0; vecX[2][7] = 0;
    vecX[3][0] = 0; vecX[3][1] = 0; vecX[3][2] = 1; vecX[3][3] = 0;
    vecX[3][4] = 0; vecX[3][5] = 0; vecX[3][6] = 1; vecX[3][7] = 0;
    vecX[4][0] = 150000; vecX[4][1] = 24000; vecX[4][2] = 12000; vecX[4][3] = 0;
    vecX[4][4] = 0; vecX[4][5] = 0; vecX[4][6] = 0; vecX[4][7] = 1;

    vecB[0][0] = 650000; vecB[1][0] = 2; vecB[2][0] = 3; vecB[3][0] = 2; vecB[4][0] = 1000000;

    // 迭代
    if(StdSimpAlg(vecMain, vecB, vecX))
    {
        std::cout << "目标矩阵" << std::endl;
        std::cout << vecMain << std::endl;
        std::cout << "系数矩阵" << std::endl;
        std::cout << vecX << std::endl;
        std::cout << "曾广矩阵" << std::endl;
        std::cout << vecB << std::endl;
    }
    // 得解
    auto matRes = GetSipAlgResult(vecB, vecX, false);
    std:: cout << "特解" << std::endl;
    std::cout << matRes << std::endl;

    std::cout << "户外广告: " << matRes[0][0] << std::endl;
    std::cout << "专业杂志: " << matRes[1][0] << std::endl;
    std::cout << "其他形式: " << matRes[2][0] << std::endl;

    auto dMax = 192 * matRes[0][0] +
        36 * matRes[1][0] +
        12 * matRes[2][0];
    std::cout << "最大值: " <<  dMax << std::endl;

    return EXIT_SUCCESS;
}