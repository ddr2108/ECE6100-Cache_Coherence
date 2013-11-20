make;
rm a;
echo "Start-4"
./sim_trace -t ./traces/4proc_validation/ -p MSI 2>>a;
diff a ./traces/4proc_validation/MSI_validation.txt;
rm a;
echo "MSI-4"
./sim_trace -t ./traces/4proc_validation/ -p MOSI 2>>a;
diff a ./traces/4proc_validation/MOSI_validation.txt;
rm a;
echo "MOSI-4"
./sim_trace -t ./traces/4proc_validation/ -p MESI 2>>a;
diff a ./traces/4proc_validation/MESI_validation.txt;
rm a;
echo "MESI-4"
./sim_trace -t ./traces/4proc_validation/ -p MOESI 2>>a;
diff a ./traces/4proc_validation/MOESI_validation.txt;
rm a;
echo "MOESI-4"
./sim_trace -t ./traces/4proc_validation/ -p MOESIF 2>>a;
diff a ./traces/4proc_validation/MOESIF_validation.txt;
rm a;
echo "MOESIF-4"
echo "Finish-4"
echo "Start-8"
./sim_trace -t ./traces/8proc_validation/ -p MSI 2>>a;
diff a ./traces/8proc_validation/MSI_validation.txt;
rm a;
echo "MSI-8"
./sim_trace -t ./traces/8proc_validation/ -p MOSI 2>>a;
diff a ./traces/8proc_validation/MOSI_validation.txt;
rm a;
echo "MOSI-8"
./sim_trace -t ./traces/8proc_validation/ -p MESI 2>>a;
diff a ./traces/8proc_validation/MESI_validation.txt;
rm a;
echo "MESI-8"
./sim_trace -t ./traces/8proc_validation/ -p MOESI 2>>a;
diff a ./traces/8proc_validation/MOESI_validation.txt;
rm a;
echo "MOESI-8"
./sim_trace -t ./traces/8proc_validation/ -p MOESIF 2>>a;
diff a ./traces/8proc_validation/MOESIF_validation.txt;
rm a;
echo "MOESIF-8"
echo "Finish-8"
echo "Start-16"
./sim_trace -t ./traces/16proc_validation/ -p MSI 2>>a;
diff a ./traces/16proc_validation/MSI_validation.txt;
rm a;
echo "MSI-16"
./sim_trace -t ./traces/16proc_validation/ -p MOSI 2>>a;
diff a ./traces/16proc_validation/MOSI_validation.txt;
rm a;
echo "MOSI-16"
./sim_trace -t ./traces/16proc_validation/ -p MESI 2>>a;
diff a ./traces/16proc_validation/MESI_validation.txt;
rm a;
echo "MESI-16"
./sim_trace -t ./traces/16proc_validation/ -p MOESI 2>>a;
diff a ./traces/16proc_validation/MOESI_validation.txt;
rm a;
echo "MOESI-16"
./sim_trace -t ./traces/16proc_validation/ -p MOESIF 2>>a;
diff a ./traces/16proc_validation/MOESIF_validation.txt;
rm a;
echo "MOESIF-16"
echo "Finish-16"