name = '105';
%%
%%
[a1,a2,a3,a4]=textread([name,'.pts'],'%f%f%f%f','headerlines',5);
a = [a1,a2,a3,a4];
%%
fileid = fopen(['assist_',name,'.txt'],'w');
%%
fprintf(fileid,'1,1,100,%d;\r\n',size(a,1));
fprintf(fileid,'0,0,0,0;\r\n');
a = round(a*100)/100;
for i=1:size(a,1)
    for j = 1:size(a,2)-1
        fprintf(fileid,'%f,',a(i,j));
    end
    fprintf(fileid,'%f;\r\n',a(i,end));
end
fclose(fileid);