//#include "shm_timevalues.h"
#include "optwins.h"
#include "k1.h"
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <unistd.h>
#include <cfloat>

typedef std::map<int, std::map<float, optwins>>::iterator opt_wins_it;
typedef std::map<float, optwins>::iterator         opt_wins_second_asc_it;
typedef std::map<float, optwins>::reverse_iterator opt_wins_second_desc_it;

optwins get_win(float dev, int id_option,  std::map<int, std::map<float, optwins>> *opt_wins){
    opt_wins_it it = opt_wins->find(id_option);
    optwins res;
    if (it != opt_wins->end()){ //если найден
        if (dev >= 0){
            for(opt_wins_second_desc_it it_sub = it->second.rbegin(); it_sub != it->second.rend(); it_sub++ ){
                if ( it_sub->first >= 0 && it_sub->first <= dev ){
                    res.win = it_sub->second.win;
                    res.id_opt1 = it_sub->second.id_opt1;
                    res.id_opt2 = it_sub->second.id_opt2;
                    break;    
                }
            }            
        } else {
            for(opt_wins_second_asc_it it_sub = it->second.begin(); it_sub != it->second.end(); it_sub++ ){
                if ( it_sub->first <= 0 && it_sub->first >= dev ){
                    res.win = it_sub->second.win;
                    res.id_opt1 = it_sub->second.id_opt1;
                    res.id_opt2 = it_sub->second.id_opt2;
                    break;    
                }
            } 
        }        
    }
    return res;    
}


int main( int argc, const char* argv[] ) {

    std::map<double, int> t_k1;
    std::map<double, int> t_k2;
    std::map<double, int> t_k3;

    t_k1[0] = 3000;
    t_k1[0.000001] = 3000;
    t_k1[0.000002] = 2000;
    t_k1[0.000003] = 1423;
    t_k1[0.000004] = 1285;
    t_k1[0.000005] = 1285;
    t_k1[0.000006] = 1275;
    t_k1[0.000007] = 1265;
    t_k1[0.000008] = 1255;
    t_k1[0.000009] = 1245;
    t_k1[0.00001]  = 1235;
    t_k1[0.000011] = 1225;
    t_k1[0.000012] = 1215;
    t_k1[0.000013] = 1000;
    t_k1[0.000014] = 1195;
    t_k1[0.000015] = 1185;
    t_k1[0.000016] = 1175;
    t_k1[0.000017] = 1165;
    t_k1[0.000018] = 1000;
    t_k1[0.000019] = 1145;
    t_k1[0.00002]  = 1135;
    t_k1[0.000021] = 1125;
    t_k1[0.000022] = 1000;
    t_k1[0.000023] = 1105;
    t_k1[0.000024] = 1095;
    t_k1[0.000025] = 1085;
    t_k1[0.000026] = 1075;
    t_k1[0.000027] = 1000;
    t_k1[0.000028] = 1055;
    t_k1[0.000029] = 1045;
    t_k1[0.00003]  = 1035;
    t_k1[0.000031] = 1000;
    t_k1[0.000032] = 1015;
    t_k1[0.000033] = 1005;
    t_k1[0.000034] = 995;
    t_k1[0.000035] = 985;
    t_k1[0.000036] = 975;
    t_k1[0.000037] = 965;
    t_k1[0.000038] = 1000;
    t_k1[0.000039] = 945;
    t_k1[0.00004]  = 935;
    t_k1[0.000041] = 925;
    t_k1[0.000042] = 915;
    t_k1[0.000043] = 905;
    t_k1[0.000044] = 895;
    t_k1[0.000045] = 1000;
    t_k1[0.000046] = 875;
    t_k1[0.000047] = 865;
    t_k1[0.000048] = 855;
    t_k1[0.000049] = 845;
    t_k1[0.00005]  = 835;
    t_k1[0.000051] = 1000;
    t_k1[0.000052] = 815;
    t_k1[0.000053] = 805;
    t_k1[0.000054] = 795;
    t_k1[0.000055] = 785;
    t_k1[0.000056] = 775;
    t_k1[0.000057] = 1000;
    t_k1[0.000058] = 755;
    t_k1[0.000059] = 745;
    t_k1[0.00006]  = 735;
    t_k1[0.000061] = 725;
    t_k1[0.000062] = 1000;
    t_k1[0.000063] = 705;
    t_k1[0.000064] = 695;
    t_k1[0.000065] = 685;
    t_k1[0.000066] = 675;
    t_k1[0.000067] = 1000;
    t_k1[0.000068] = 655;
    t_k1[0.000069] = 645;
    t_k1[0.00007]  = 635;
    t_k1[0.000071] = 625;
    t_k1[0.000072] = 900;
    t_k1[0.000073] = 605;
    t_k1[0.000074] = 595;
    t_k1[0.000075] = 585;
    t_k1[0.000076] = 800;
    t_k1[0.000077] = 565;
    t_k1[0.000078] = 555;
    t_k1[0.000079] = 545;
    t_k1[0.00008]  = 700;
    t_k1[0.000081] = 525;
    t_k1[0.000082] = 515;
    t_k1[0.000083] = 600;
    t_k1[0.000084] = 495;
    t_k1[0.000085] = 485;
    t_k1[0.000086] = 475;
    t_k1[0.000087] = 465;
    t_k1[0.000088] = 500;
    t_k1[0.000089] = 445;
    t_k1[0.00009]  = 435;
    t_k1[0.000091] = 425;
    t_k1[0.000092] = 500;
    t_k1[0.000093] = 405;
    t_k1[0.000094] = 395;
    t_k1[0.000095] = 385;
    t_k1[0.000096] = 400;
    t_k1[0.000097] = 365;
    t_k1[0.000098] = 355;
    t_k1[0.000099] = 345;
    t_k1[0.0001]   = 400;
    t_k1[0.000101] = 325;
    t_k1[0.000102] = 315;
    t_k1[0.000103] = 305;
    t_k1[0.000104] = 300;
    t_k1[0.000105] = 285;
    t_k1[0.000106] = 275;
    t_k1[0.000107] = 265;
    t_k1[0.000108] = 300;
    t_k1[0.000109] = 245;
    t_k1[0.00011]  = 235;
    t_k1[0.000111] = 225;
    t_k1[0.000112] = 300;
    t_k1[0.000113] = 215;
    t_k1[0.000114] = 210;
    t_k1[0.000115] = 205;
    t_k1[0.000116] = 200;
    t_k1[0.000117] = 195;
    t_k1[0.000118] = 190;
    t_k1[0.000119] = 185;
    t_k1[0.00012]  = 200;
    t_k1[0.000121] = 175;
    t_k1[0.000122] = 170;
    t_k1[0.000123] = 165;
    t_k1[0.000124] = 200;
    t_k1[0.000125] = 155;
    t_k1[0.000126] = 150;
    t_k1[0.000127] = 145;
    t_k1[0.000128] = 100;
    t_k1[0.000129] = 135;
    t_k1[0.00013]  = 130;
    t_k1[0.000131] = 125;
    t_k1[0.000132] = 100;
    t_k1[0.000133] = 115;
    t_k1[0.000134] = 110;
    t_k1[0.000135] = 105;
    t_k1[0.000136] = 100;
    t_k1[0.000137] = 95;
    t_k1[0.000138] = 90;
    t_k1[0.000139] = 85;
    t_k1[0.00014]  = 100;
    t_k1[0.000141] = 75;
    t_k1[0.000142] = 70;
    t_k1[0.000143] = 65;
    t_k1[0.000144] = 100;
    t_k1[0.000145] = 58;
    t_k1[0.000146] = 56;
    t_k1[0.000147] = 54;
    t_k1[0.000148] = 100;
    t_k1[0.000149] = 50;
    t_k1[0.00015]  = 48;
    t_k1[0.000151] = 46;
    t_k1[0.000152] = 100;
    t_k1[0.000153] = 42;
    t_k1[0.000154] = 40;
    t_k1[0.000155] = 38;
    t_k1[0.000156] = 100;
    t_k1[0.000157] = 34;
    t_k1[0.000158] = 32;
    t_k1[0.000159] = 30;
    t_k1[0.00016]  = 100;
    t_k1[0.000161] = 26;
    t_k1[0.000162] = 24;
    t_k1[0.000163] = 22;
    t_k1[0.000164] = 100;
    t_k1[0.000165] = 19;
    t_k1[0.000166] = 18;
    t_k1[0.000167] = 17;
    t_k1[0.000168] = 100;
    t_k1[0.000169] = 15;
    t_k1[0.00017]  = 14;
    t_k1[0.000171] = 13;
    t_k1[0.000172] = 100;
    t_k1[0.000173] = 11;
    t_k1[0.000174] = 10;
    t_k1[0.000175] = 9;
    t_k1[0.000176] = 100;
    t_k1[0.000177] = 7;
    t_k1[0.000178] = 6;
    t_k1[0.000179] = 5;
    t_k1[0.00018]  = 100;
    t_k1[0.000181] = 5;
    t_k1[0.000182] = 5;
    t_k1[0.000183] = 5;
    t_k1[0.000184] = 100;
    t_k1[0.000185] = 5;
    t_k1[0.000186] = 5;
    t_k1[0.000187] = 5;
    t_k1[0.000188] = 100;
    t_k1[0.000189] = 1;
    t_k1[0.00019]  = 1;
    t_k1[0.000191] = 1;
    t_k1[0.000192] = 100;
    t_k1[0.000193] = 1;
    t_k1[0.000194] = 1;
    t_k1[0.000195] = 100;
    t_k1[0.000196] = 1;
    t_k1[0.000197] = 1;
    t_k1[0.000198] = 1;


    t_k2[0.000013] = 1;
    t_k2[0.000018] = 1;
    t_k2[0.000022] = 1;
    t_k2[0.000027] = 1;
    t_k2[0.000031] = 1;
    t_k2[0.000038] = 1;
    t_k2[0.000045] = 1;
    t_k2[0.000051] = 1;
    t_k2[0.000057] = 1;
    t_k2[0.000062] = 1;
    t_k2[0.000067] = 1;
    t_k2[0.000072] = 1;
    t_k2[0.000076] = 1;
    t_k2[0.00008]  = 1;
    t_k2[0.000083] = 1;
    t_k2[0.000088] = 1;
    t_k2[0.000092] = 1;
    t_k2[0.000096] = 1;
    t_k2[0.0001]   = 1;
    t_k2[0.000104] = 1;
    t_k2[0.000108] = 1;
    t_k2[0.000112] = 1;
    t_k2[0.000116] = 1;
    t_k2[0.00012]  = 1;
    t_k2[0.000124] = 1;
    t_k2[0.000128] = 1;
    t_k2[0.000132] = 1;
    t_k2[0.000136] = 1;
    t_k2[0.00014]  = 1;
    t_k2[0.000144] = 1;
    t_k2[0.000148] = 1;
    t_k2[0.000152] = 1;
    t_k2[0.000156] = 1;
    t_k2[0.00016]  = 1;
    t_k2[0.000164] = 1;
    t_k2[0.000168] = 1;
    t_k2[0.000172] = 1;
    t_k2[0.000176] = 1;
    t_k2[0.00018]  = 1;
    t_k2[0.000184] = 1;
    t_k2[0.000188] = 1;
    t_k2[0.000192] = 1;
    t_k2[0.000195] = 1;
    t_k2[0.000198] = 1;
    
    t_k3[0.00005]  = 1;
    t_k3[0.000049] = 1;
    t_k3[0.000048] = 1;
    t_k3[0.000047] = 1;
    t_k3[0.000046] = 1;
    t_k3[0.000045] = 1;
    t_k3[0.000044] = 1;
    t_k3[0.000043] = 1;
    t_k3[0.000042] = 1;
    t_k3[0.000041] = 1;
    t_k3[0.00004]  = 1;
    t_k3[0.000039] = 1;
    t_k3[0.000038] = 1;
    t_k3[0.000037] = 1;
    t_k3[0.000036] = 1;
    t_k3[0.000035] = 1;
    t_k3[0.000034] = 1;
    t_k3[0.000033] = 1;
    t_k3[0.000032] = 1;
    t_k3[0.000031] = 1;
    t_k3[0.00003]  = 1;
    t_k3[0.000029] = 1;
    t_k3[0.000028] = 1;
    t_k3[0.000027] = 1;
    t_k3[0.000026] = 1;
    t_k3[0.000025] = 1;
    t_k3[0.000024] = 1;
    t_k3[0.000023] = 1;
    t_k3[0.000022] = 1;
    t_k3[0.000021] = 1;
    t_k3[0.00002]  = 1;
    t_k3[0.000019] = 1;
    t_k3[0.000018] = 1;
    t_k3[0.000017] = 1;
    t_k3[0.000016] = 1;
    t_k3[0.000015] = 1;
    t_k3[0.000014] = 1;
    t_k3[0.000013] = 1;
    t_k3[0.000012] = 1;
    t_k3[0.000011] = 1;
    t_k3[0.00001]  = 1;
    t_k3[0.000009] = 1;
    t_k3[0.000008] = 1;
    t_k3[0.000007] = 1;
    t_k3[0.000006] = 1;
    t_k3[0.000005] = 1;
    t_k3[0.000004] = 1;
    t_k3[0.000003] = 1;
    t_k3[0.000002] = 1;
    t_k3[0.000001] = 1;
    
    
    int k1_size = 0;
    int k2_size = 0;
    int k3_size = 0;
    for ( auto it : t_k1 ){
        k1_size += it.second;
    }
    for ( auto it : t_k2 ){
        k2_size += it.second;
    }
    for ( auto it : t_k3 ){
        k3_size += it.second;
    }

    double k1[k1_size];
    double k2[k2_size];
    double k3[k3_size];
    
    //std::cout << k1_size << std::endl;
    //std::cout << k2_size << std::endl;
    //std::cout << k3_size << std::endl;
    
    int first = 0;
    int last = 0;
    for ( auto it : t_k1 ){
        first = last;
        last += it.second;
        for (int i=first; i<last; i++){
            k1[i] = it.first;
        }
    }
    
    first = 0;
    last = 0;
    for ( auto it : t_k2 ){
        first = last;
        last += it.second;
        for (int i=first; i<last; i++){
            k2[i] = it.first;
        }
    }
    
    first = 0;
    last = 0;
    for ( auto it : t_k3 ){
        first = last;
        last += it.second;
        for (int i=first; i<last; i++){
            k3[i] = it.first;
        }
    }
    

    
    srand (time(NULL));

    std::random_shuffle(&k1[0], &k1[k1_size-1]);
    std::random_shuffle(&k2[0], &k1[k2_size-1]);
    std::random_shuffle(&k3[0], &k1[k3_size-1]);

    double BA1 = 1.1153;

    double BA2;
    
    

    double NBA1 = BA1;
    double NBA2 = -1;
    
    long count = 0;
    for (;;){
        count++;
        if (count > 1000000){
            break;
        }

        int R1 = rand() % 99;

        if (R1 >= 0 && R1 < 79){
            BA2 = BA1;
        }

        if (R1 >= 79 && R1 < 94 ){
            if (rand() % 99 < 50)
                BA2 = BA1 - 0.0001;
            else
                BA2 = BA1 + 0.0001;
        }

        if (R1 >= 95 && R1 <= 99){
            if (rand() % 99 < 50)
                BA2 = BA1 - 0.0002;
            else
                BA2 = BA1 + 0.0002;
        }

        std::random_shuffle(&k1[0], &k1[k1_size-1]);
        double K = 0;
        
        double my = (BA1 - BA2); 
        
        if ( fabs(my) <= DBL_EPSILON ){
            //std::cout << "=:";
            int iter = 0;
            int iter_max = 1000;
            for(;;){
                iter++;
                K = k1[rand() % k1_size];
                if (NBA1 > BA2)                         NBA2 = (rand() % 99 < 70)? NBA1 - K : NBA1 + K; 
                if (NBA1 < BA2)                         NBA2 = (rand() % 99 < 70)? NBA1 + K : NBA1 - K;                    
                if (fabs(NBA1 - BA2) <= DBL_EPSILON)    NBA2 = (rand() % 99 < 50)? NBA1 - K : NBA1 + K;  

                if  ( !(NBA2 > (BA2 + 0.000099) || NBA2 < (BA2 - 0.000099))){
                    break;
                }
                if ( iter == iter_max ) {
                    NBA2 = NBA1;
                    break;
                }                
            } 
        }
        
        
        if ( my > 0 && my <= 0.0001 ) {
            //std::cout << "=+0.0001:";            
            int iter = 0;
            int iter_max = 3;
            for(;;){
                K = k1[rand() % k1_size];
                NBA2 = NBA1 - K;
                if  ( NBA2 > (BA2 + 0.000099) || NBA2 < (BA2 - 0.000099)) {
                    iter++;
                    if (iter == iter_max) {
                        int iter = 0;
                        int iter_max = 1000;
                        for(;;){
                            iter++;
                            K = k2[rand() % k2_size];
                            NBA2 = NBA1 - K;
                            if  (!(NBA2 > (BA2 + 0.000099) || NBA2 < (BA2 - 0.000099))){
                                break;
                            }
                            if (iter == iter_max){
                                NBA2 = NBA1;
                                break;
                            }
                        }
                        break;
                    }
                } else {
                    break;
                }
            } 
        } 
        
        if ( my < 0 && my >= -0.0001 ) {
            //std::cout << "=-0.0001:";
            int iter = 0;
            int iter_max = 3;
            for(;;){
                K = k1[rand() % k1_size];
                NBA2 = NBA1 + K;
                if  ( NBA2 > (BA2 + 0.000099) || NBA2 < (BA2 - 0.000099)) {
                    iter++;
                    if (iter == iter_max) {
                        int iter = 0;
                        int iter_max = 1000;
                        for(;;){
                            iter++;
                            K = k2[rand() % k2_size];
                            NBA2 = NBA1 + K;
                            if  (!(NBA2 > (BA2 + 0.000099) || NBA2 < (BA2 - 0.000099))){
                                break;
                            }
                            if (iter == iter_max){
                                NBA2 = NBA1;
                                break;
                            }
                        }
                        break;
                    }
                    
                } else {
                    break;
                }
            } 
        } 
        
        if ( my > 0.0001 ){
            //std::cout << ">+0.0001:";
            int iter = 0;
            int iter_max = 1000;
            for(;;){
                iter++;
                K = k3[rand() % k3_size];
                NBA2 = NBA1 - (BA1-BA2) - K;
                
                if  ( !(NBA2 > (BA2 + 0.000099) || NBA2 < (BA2 - 0.000099))){
                    break;
                }
                if ( iter == iter_max ) {
                    NBA2 = BA1;
                    break;
                }                
            } 
        }  
        
        if ( my < -0.0001 ){
            //std::cout << "<-0.0001:";            
            int iter = 0;
            int iter_max = 1000;
            for(;;){
                iter++;
                K = k3[rand() % k3_size];
                NBA2 = NBA1 + (BA1-BA2) + K;
                
                if  ( !(NBA2 > (BA2 + 0.000099) || NBA2 < (BA2 - 0.000099))){
                    break;
                }
                if ( iter == iter_max ) {
                    NBA2 = BA1;
                    break;
                }                
            } 
        }                 
        
        std::cout.precision(6);
        //std::cout << "NBA2 = " << std::fixed <<  NBA2 << "; " << "K = " << K << std::endl;
        std::cout << std::fixed <<  NBA2 << std::endl;  
        
        NBA1 = NBA2;
        //BA1 = BA2;
        //sleep(1);

    }    
    
    
    /*
    for (int i=0;i<k1_size;i++){
        std::cout << k1[i] << std::endl;
    }
    */
    
    /*
    std::map<int, std::map<float, optwins>> opt_wins;
    //Опцион, базис
    std::map<int, int> opt_basis;
    
    opt_basis[107] = 10;
    opt_basis[108] = 10;
    opt_basis[109] = 10;
    opt_basis[110] = 15;
    
    for ( auto it : opt_basis ){
        std::cout << it.first << "\t" << it.second << std::endl; 
    }
    
    std::cout << std::endl;  
    // выплаты по опционам
    optwins my;
    my.win = 10;
    my.id_opt1 = 0;
    my.id_opt2 = 0;
    //opt_wins[107][100]   = {10, 0, 0};
    opt_wins[107][100]   = my;
    opt_wins[107][90]    = {5,  0, 0};
    opt_wins[107][60]    = {2,  0, 0};
    opt_wins[107][50]    = {1,  0, 0};
    opt_wins[107][0]     = {0,  0, 0};
    opt_wins[107][-50]   = {1,  108, 0};
    opt_wins[107][-99]   = {2,  0, 0};
    opt_wins[107][-100]  = {5,  0, 0};
    opt_wins[107][-200]  = {10, 0, 0};

    float dev = -51;
    optwins res = get_win(dev, 107, &opt_wins);
    
    std::cout << "\t" << dev << "\t" << res.win << "\t" << res.id_opt1 << "\t" << res.id_opt2 << std::endl;
    */
    //std::cout << "END" << std::endl;
    return 0;
	
}
