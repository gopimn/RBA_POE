clear all;
clc;
clear all;
adq_timer = timer('TimerFcn', 'stat=false; disp(''Timer!'')','StartDelay',60);
stat=true;
t = tcpip('0.0.0.0', 2032, 'NetworkRole', 'server');
fopen(t);
data_actual = fscanf(t,'%f;%f;%f;%f');
data_array=data_actual';
i=0;
start(adq_timer);
while (stat==true)
data_actual=fscanf(t,'%f;%f;%f;%f');
disp(data_actual);
data_array=cat(1,data_array,data_actual');
end
fclose(t);
ID=fopen('test3.txt','w');
for i=1:size(data_array,1)
fprintf(ID,'%f %f %f %f\n',data_array(i,1),data_array(i,2),data_array(i,3),data_array(i,4));
end
figure,hold on,
plot(data_array(:,1),data_array(:,2).*0.000000487675,...
data_array(:,1),data_array(:,3).*0.000000487675,...    
data_array(:,1),data_array(:,4).*0.000000487675);
