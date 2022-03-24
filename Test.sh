# for i in {0..100}
# do
#     echo ""
#     echo ""
#     echo "Starting test $i"

#   ./multi-lookup 6 0 seviced.txt resolved.txt input/names1*.txt

#   sleep .5
# done

# for i in {0...10}
# do
#   echo "$i"

#   #eval ./multi-lookup i i serviced.txt resolved.txt input/*

#   echo ""

#   sleep .5
# done
make
clear
for i in {0..10}
do
    echo ""
    echo ""
    echo "Starting test $i"

    eval './multi-lookup $i $i serviced.txt resolved.txt input/*'

  #./multi-lookup 6 0 seviced.txt resolved.txt input/names1*.txt

  sleep .5
done