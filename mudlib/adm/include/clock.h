#define THE_BEGINNING 720561600
#define NOW(x) x-THE_BEGINNING

#define SECOND 1
#define MINUTE 8
#define HOUR 480
#define DAY 11520
#define WEEK 80640
#define MONTH 4193280
#define YEAR 50319360

#define CURRENT_YEAR(x) (1+((NOW(x))/(YEAR)))
#define CURRENT_MONTH(x) ((NOW(x))%YEAR)/MONTH
#define CURRENT_WEEK(x) ((NOW(x))%MONTH)/WEEK
#define CURRENT_DAY(x) ((NOW(x))%WEEK)/DAY
#define CURRENT_DATE(x) (1+((NOW(x))%MONTH)/DAY)
#define CURRENT_HOUR(x) ((NOW(x))%DAY)/HOUR
#define CURRENT_MINUTE(x) ((NOW(x))%HOUR)/MINUTE
#define CURRENT_SECOND(x) ((NOW(x))%MINUTE)*MINUTE

#define LONGEST_DAY 101
#define SHORTEST_DAY 1

#define DAYS ({ "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" })
#define MONTHS ({ "January", "February", "March", "April", "May",\
"June", "July", "August", "September", "October", "November", "December" })
