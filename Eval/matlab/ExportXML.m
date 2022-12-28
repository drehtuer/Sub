function ExportXML( sDirectory, DataStruct, ComparisonsStruct )
%EXPORTXML Summary of this function goes here
%   Detailed explanation goes here

%% data
fid = fopen([sDirectory filesep 'Data.csv'], 'w');
fprintf(fid, 'key,age,sex,skill\n');

for i=1:length(DataStruct)
   %disp([DataStruct(i).sKey ' ' int2str(DataStruct(i).iAge) ' ' int2str(DataStruct(i).iSex) ' ' int2str(DataStruct(i).iSkill)]);
   fprintf(fid, '%s,%d,%d,%d\n', DataStruct(i).sKey, DataStruct(i).iAge, DataStruct(i).iSex, DataStruct(i).iSkill);
end

fclose(fid);

%% comparisons

fid = fopen([sDirectory filesep 'Comparisons.csv'], 'w');
fprintf(fid, 'Head,,,,,,,,,,,,ArmOpen,,,,,,,,,,,,ArmClosed,,,,,,,,,,,\n');
fprintf(fid, 'behind,,,,,,side,,,,,,behind,,,,,,side,,,,,,behind,,,,,,side,,,,,\n');
fprintf(fid, 'light far,,,light near,,,light far,,,light near,,,light far,,,light near,,,light far,,,light near,,,light far,,,light near,,,light far,,,light near,,\n');
fprintf(fid, 'dEon,Hable,Brand,dEon,Hable,Brand,dEon,Hable,Brand,dEon,Hable,Brand,dEon,Hable,Brand,dEon,Hable,Brand,dEon,Hable,Brand,dEon,Hable,Brand,dEon,Hable,Brand,dEon,Hable,Brand,dEon,Hable,Brand,dEon,Hable,Brand\n');

[numParticipants, ~] = size(ComparisonsStruct);
for participant=1:numParticipants
for model=1:3
    for dir=1:2
        for dist=1:2
            for algo=2:4 % 1 is jimenez
                fprintf(fid, '%d', print(ComparisonsStruct(participant,:), model, dir, dist, algo));
                if model ~= 3 || dir ~= 2 || dist ~= 2 || algo ~= 4
                    fprintf(fid, ',');
                end
            end
        end
    end
end
fprintf(fid, '\n');
end

fclose(fid);

end

function [ iChoice ] = print(Comparison, iModelId, iLightDirectionId, iLightDistanceId, iAlgorithmId)
for i=1:length(Comparison)
    if Comparison(i).iModelId == iModelId && Comparison(i).iLightDirectionId == iLightDirectionId && Comparison(i).iLightDistanceId == iLightDistanceId && (Comparison(i).iFirstAlgorithmId == iAlgorithmId || Comparison(i).iSecondAlgorithmId == iAlgorithmId)
        iChoice = Comparison(i).iChoice;
        break;
    end
end
end
