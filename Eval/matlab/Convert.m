function Convert( sXmlDirectory, sExportDirectory )
%CONVERT Summary of this function goes here
%   Detailed explanation goes here

% get all xml files
XMLs = dir([sXmlDirectory filesep '*.xml']);
[numXmls, ~] = size(XMLs);
disp(['Found ' int2str(numXmls) ' files']);

iRecerenceAlgorithmId = 1;

% for all xml
for i=1:numXmls
    [D, C] = ReadXML([sXmlDirectory filesep XMLs(i).name], iRecerenceAlgorithmId);
    Data(i).sKey = D.sKey;
    Data(i).iAge = D.iAge;
    Data(i).iSex = D.iSex;
    Data(i).iSkill = D.iSkill;
    for j=1:length(C)
        Comparisons(i, j).iModelId = C(j).iModelId;
        Comparisons(i, j).iChoice = C(j).iChoice;
        Comparisons(i, j).iFirstAlgorithmId = C(j).iFirstAlgorithmId;
        Comparisons(i, j).iSecondAlgorithmId = C(j).iSecondAlgorithmId;
        Comparisons(i, j).iLightDirectionId = C(j).iLightDirectionId;
        Comparisons(i, j).iLightDistanceId = C(j).iLightDistanceId;
    end
end

% export
ExportXML(sExportDirectory, Data, Comparisons);

end

