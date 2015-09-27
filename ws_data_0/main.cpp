//#include "shm_timevalues.h"
#include "optwins.h"
#include <iostream>
#include <map>

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
/*
    managed_shared_memory segment(open_only,"ws_server_data");
    TimeValues_set *es = segment.find<TimeValues_set>("TimeValues").first;
    auto it = --es->end();
    std::cout << it->time << "\t" << it->value1   << "\t"; std::cout << std::endl;
*/  

    //std::map<int, std::map<float, float>> opt_wins;
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

/*
    opt_wins[107][100]   = 10;
    opt_wins[107][90]    = 5;
    opt_wins[107][60]    = 2;
    opt_wins[107][50]    = 1;
    opt_wins[107][0]     = 0;
    opt_wins[107][-50]   = 1;
    opt_wins[107][-99]   = 2;
    opt_wins[107][-100]  = 5;
    opt_wins[107][-200]  = 10;
*/
    /*
    for ( auto it : opt_wins ){
        std::cout << it.first << std::endl;
        for ( auto it_sub : it.second ){
            std::cout << "\t" << it_sub.first << "\t" << it_sub.second << std::endl;
        }
    } 
*/
/*
	IF dev >= 0 THEN
		SELECT p01 FROM boptwins WHERE p01 >= 0 AND p01 <= dev ORDER BY p01 DESC LIMIT 1 INTO res;
	ELSE
		SELECT p01 FROM boptwins WHERE p01 <= 0 AND p01 >= dev ORDER BY p01 ASC  LIMIT 1 INTO res;
	END IF;
*/
/*
    //по опционам:
    for ( auto it : opt_wins ){
        std::cout << it.first << std::endl;
        //по коридорам
        for ( auto it_sub : it.second ){
            if ( my <= it_sub.first ){
                std::cout << my << "\t" << "\t" << it_sub.first << "\t" << it_sub.second << std::endl;
                break;
            }
        }
    } 
*/
/*
    float dev = -51;
    
    typedef std::map<int, std::map<float, float>>::iterator opt_wins_it;
    typedef std::map<float, float>::iterator         opt_wins_second_asc_it;
    typedef std::map<float, float>::reverse_iterator opt_wins_second_desc_it;
        
    for(opt_wins_it it = opt_wins.begin(); it != opt_wins.end(); it++) {    
        std::cout << it->first << std::endl;
        for(opt_wins_second_asc_it it_sub = it->second.begin(); it_sub != it->second.end(); it_sub++ ){
            std::cout << "\t" << it_sub->first << "\t" << it_sub->second << std::endl;
        }
    }
    
    std::cout << std::endl;
    opt_wins_it it = opt_wins.find(107);
    
    float my_dev;
    float my_win;
    
    if (it != opt_wins.end()){ //если найден
        if (dev >= 0){
            for(opt_wins_second_desc_it it_sub = it->second.rbegin(); it_sub != it->second.rend(); it_sub++ ){
                if ( it_sub->first >= 0 && it_sub->first <= dev ){
                    my_dev = it_sub->first;
                    my_win = it_sub->second;
                    break;    
                }
            }            
        } else {
            for(opt_wins_second_asc_it it_sub = it->second.begin(); it_sub != it->second.end(); it_sub++ ){
                if ( it_sub->first <= 0 && it_sub->first >= dev ){
                    my_dev = it_sub->first;
                    my_win = it_sub->second;
                    break;    
                }
            } 
        }        
    }
 
    std::cout << "\t" << my_dev << "\t" << my_win << std::endl;
*/    
    /*
    for(opt_wins_it it = opt_wins.begin(); it != opt_wins.end(); it++) {    
        std::cout << it->first << std::endl;
        if (dev >= 0){
            for(opt_wins_second_desc_it it_sub = it->second.rbegin(); it_sub != it->second.rend(); it_sub++ ){
                if ( it_sub->first >= 0 && it_sub->first <= dev ){
                    std::cout << "\t" << it_sub->first << "\t" << it_sub->second << std::endl;
                    break;    
                }
            }            
        } else {
            for(opt_wins_second_asc_it it_sub = it->second.begin(); it_sub != it->second.end(); it_sub++ ){
                if ( it_sub->first <= 0 && it_sub->first >= dev ){
                    std::cout << "\t" << it_sub->first << "\t" << it_sub->second << std::endl;
                    break;    
                }
            } 
        }
    } 
    */   
    /*
    float dev = -51;
    
    typedef std::map<int, std::map<float, optwins>>::iterator opt_wins_it;
    typedef std::map<float, optwins>::iterator         opt_wins_second_asc_it;
    typedef std::map<float, optwins>::reverse_iterator opt_wins_second_desc_it;
        
    for(opt_wins_it it = opt_wins.begin(); it != opt_wins.end(); it++) {    
        std::cout << it->first << std::endl;
        for(opt_wins_second_asc_it it_sub = it->second.begin(); it_sub != it->second.end(); it_sub++ ){
            std::cout 
            << "\t" << it_sub->first 
            << "\t" << it_sub->second.win
            << "\t" << it_sub->second.id_opt1
            << "\t" << it_sub->second.id_opt2 
            << std::endl;
        }
    }
    
    std::cout << std::endl;
    opt_wins_it it = opt_wins.find(107);
    
    float my_dev;
    float my_win;
    
    if (it != opt_wins.end()){ //если найден
        if (dev >= 0){
            for(opt_wins_second_desc_it it_sub = it->second.rbegin(); it_sub != it->second.rend(); it_sub++ ){
                if ( it_sub->first >= 0 && it_sub->first <= dev ){
                    my_dev = it_sub->first;
                    my_win = it_sub->second.win;
                    break;    
                }
            }            
        } else {
            for(opt_wins_second_asc_it it_sub = it->second.begin(); it_sub != it->second.end(); it_sub++ ){
                if ( it_sub->first <= 0 && it_sub->first >= dev ){
                    my_dev = it_sub->first;
                    my_win = it_sub->second.win;
                    break;    
                }
            } 
        }        
    }
 
    std::cout << "\t" << my_dev << "\t" << my_win << std::endl;    
    */
    float dev = -51;
    optwins res = get_win(dev, 107, &opt_wins);
    
    std::cout << "\t" << dev << "\t" << res.win << "\t" << res.id_opt1 << "\t" << res.id_opt2 << std::endl;
    
    std::cout << "END" << std::endl;
    return 0;
	
}
