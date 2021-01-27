/* C++ 17 file
 * Date 20201124
 * Author Liao
 */

#include "sqltools"
#include "matrix"

const db_info login("DSSW", "sa", "123456");

// 因变量矩阵
matrix GetVecY()
{
    auto rsY = sql_server::go(login, L"select sales from ex1");
    auto rsCol = rsY.get_result("sales");
    auto pVecY = new double[rsCol.size()];
    for(UINT i=0; i<rsCol.size(); i++)
        pVecY[i] = _ttof(rsCol[i]);
    matrix vecY(pVecY, rsCol.size(), 1);
    return vecY;
}

// 自变量矩阵
matrix GetVecSpcX()
{
    auto rsX = sql_server::go(login, L"select price, expenses, production from ex1");
    matrix vecSpcX(rsX.get_row_count(), rsX.get_column_count() + 1);
    for(UINT i=0; i<rsX.get_row_count(); i++)
    {
        UINT j = 0;
        vecSpcX[i][j++] = 1;
        vecSpcX[i][j++] = _ttof(rsX.get_result(i)[L"price"]);
        vecSpcX[i][j++] = _ttof(rsX.get_result(i)[L"expenses"]);
        vecSpcX[i][j++] = _ttof(rsX.get_result(i)[L"production"]);
    }
    return vecSpcX;
}

// 获取系数与截距矩阵
matrix GetRegCoe(matrix vecSpcX, matrix vecY)
{
    auto vecItr = matrix::transposition(vecSpcX) * vecSpcX;
    return (vecItr^(-1)) * (matrix::transposition(vecSpcX) * vecY);
}

// 因变量均值
double GetVecYAvg(matrix vecY)
{
    double dSumY = 0;
    for(UINT i=0; i<vecY.get_line(); i++) dSumY += vecY[i][0];
    return dSumY * 1.0 / vecY.get_line();
}

// 获取TSS
double GetRegTSS(matrix vecY)
{
    double dAvgY = GetVecYAvg(vecY), dSumTSS = 0;
    for(UINT i=0; i<vecY.get_line(); i++)
        dSumTSS += power(vecY[i][0] - dAvgY, 2);
    return dSumTSS;
}

// 获取ESS
double GetRegESS(matrix vecCoe, matrix vecSpcX, matrix vecY)
{
    auto len = vecY.get_line();
    auto pdRegY = new double[len];
    double dSumESS = 0.0, dAvgY = GetVecYAvg(vecY);
    for(UINT i=0; i<len; i++)
    {
        auto dtRegY = vecSpcX[i][1] * vecCoe[1][0] +
            vecSpcX[i][2] * vecCoe[2][0] +
            vecSpcX[i][3] * vecCoe[3][0] + vecCoe[0][0];
        dSumESS += power(dtRegY - dAvgY, 2);
    }
    delete []pdRegY;
    pdRegY = nullptr;
    return dSumESS;
}

// 获取RSS
double GetRegRSS(matrix vecCoe, matrix vecSpcX, matrix vecY)
{
    return GetRegTSS(vecY) - GetRegESS(vecCoe, vecSpcX, vecY);
}

// 拟合优度检测
double GetRegSqR(matrix vecCoe, matrix vecSpcX, matrix vecY)
{
    return GetRegESS(vecCoe, vecSpcX, vecY) / GetRegTSS(vecY);
}

int main(int argc, char **argv, char **envp)
{
    std::cout << "Hello World!" << std::endl;
    auto vecX = GetVecSpcX(), vecY = GetVecY();
    auto vecCoe = GetRegCoe(vecX, vecY);
    std::cout << "截距: " << vecCoe[0][0] << std::endl;
    std::cout << "系数: [";
    for(UINT i=0; i<3; i++)
    {
        std::cout << vecCoe[i+1][0];
        if(i+1 != 3) std::cout << ", ";
    }
    std::cout << ']' << std::endl;
    std::cout << "方程: " << "y = " <<
        vecCoe[1][0] << "x1 + " <<
        vecCoe[2][0] << "x2 + " <<
        vecCoe[3][0] << "x3 + " <<
        vecCoe[0][0] << std::endl;
    std::cout << "优度: " << GetRegSqR(vecCoe, vecX, vecY) << std::endl;
    double dNewSales = 265.32130 * (1 + 0.1),
        price = 41.26383, production = 157.00, expenses = 114.18541, 
        dNewAd = (dNewSales - vecCoe[0][0] - price * vecCoe[1][0] - production *vecCoe[3][0]) / vecCoe[2][0],
        dNewPrice = (dNewSales - vecCoe[0][0] - expenses * vecCoe[2][0] - production *vecCoe[3][0]) / vecCoe[1][0];
    std::cout << "广告: " << dNewAd << std::endl;
    std::cout << "价格: " << dNewPrice << std::endl;
    return EXIT_SUCCESS;
}