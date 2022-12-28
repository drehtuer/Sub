function [Statistics] = EvalEval( sXmlFolder )
%EVALEVAL Reads a directory, checks for eval xmls and evaluates them

% init

% per model
Statistics.Model(getModelId('Head')) = createCollector();
Statistics.Model(getModelId('HandOpen')) = createCollector();
Statistics.Model(getModelId('HandClosed')) = createCollector();
Statistics.Model(getModelId('Head')).Model = 'Head';
Statistics.Model(getModelId('HandOpen')).Model = 'HandOpen';
Statistics.Model(getModelId('HandClosed')).Model = 'HandClosed';

% per lighting conditions
for lid=1:2
    for cid=1:2
        Statistics.Light(lid, cid) = createCollector();
    end
end
[lid, cid] = getLightPositionId('side', 'near');
Statistics.Light(lid, cid).LightDirection = 'side';
Statistics.Light(lid, cid).LightDistance = 'near';

[lid, cid] = getLightPositionId('side', 'far');
Statistics.Light(lid, cid).LightDirection = 'side';
Statistics.Light(lid, cid).LightDistance = 'far';

[lid, cid] = getLightPositionId('behind', 'near');
Statistics.Light(lid, cid).LightDirection = 'behind';
Statistics.Light(lid, cid).LightDistance = 'near';

[lid, cid] = getLightPositionId('behind', 'far');
Statistics.Light(lid, cid).LightDirection = 'behind';
Statistics.Light(lid, cid).LightDistance = 'far';

% total
Statistics.Total = createCollector();

Statistics.Total.WinnerFirst = 0;
Statistics.Total.WinnerSecond = 0;
Statistics.Total.WinnerRatio = 0;

Statistics.Total.Age = zeros(1, 99);
Statistics.Total.Sex = zeros(1, 2);
Statistics.Total.Skill = zeros(1, 4);

% get all xml files
XMLs = dir([sXmlFolder filesep '*.xml']);
[numXmls, ~] = size(XMLs);
disp(['Found ' int2str(numXmls) ' files']);

% for all files
for i=1:numXmls
    %% read xml
    RootNode = xmlread([sXmlFolder filesep XMLs(i).name]);
    ResultsNode = RootNode.getElementsByTagName('Results');
    % id
    IdNode = ResultsNode.item(0).getElementsByTagName('Id');
    sKey = char(IdNode.item(0).getAttribute('key'));
    disp(['Id: ' sKey]);
    % questionaire
    QuestionnaireNode = ResultsNode.item(0).getElementsByTagName('Questionaire');
    AnswerNode = QuestionnaireNode.item(0).getElementsByTagName('Answer');
    
    iAge = str2num(char(AnswerNode.item(0).getFirstChild().getData())); %#ok<ST2NM>
    sSex = char(AnswerNode.item(1).getFirstChild().getData());
    sSkill = char(AnswerNode.item(2).getFirstChild().getData());
    
    Statistics.Total.Age(iAge) = Statistics.Total.Age(iAge) + 1;
    if(strcmp(sSex, 'm') == 1)
    	Statistics.Total.Sex(1) = Statistics.Total.Sex(1) + 1;
    else
    	Statistics.Total.Sex(2) = Statistics.Total.Sex(2) + 1;
    end
    iSkillId = getSkillId(sSkill);
    Statistics.Total.Skill(iSkillId) = Statistics.Total.Skill(iSkillId) + 1;
    
    disp(['Age ' num2str(iAge) ', Sex: ' sSex ', Skill: ' sSkill]);
    
    %% comparisons
    ComparisonNode = ResultsNode.item(0).getElementsByTagName('Comparison');
    for c=0:ComparisonNode.getLength()-1
        % for all comparisons
        sFirst = char(ComparisonNode.item(c).getElementsByTagName('First').item(0).getFirstChild().getData());
        sFirstAlgorithm = getAlgorithmName(sFirst);
        iFirstAlgorithmId = getAlgorithmId(sFirstAlgorithm);
        sSecond = char(ComparisonNode.item(c).getElementsByTagName('Second').item(0).getFirstChild().getData());
        sSecondAlgorithm = getAlgorithmName(sSecond);
        iSecondAlgorithmId = getAlgorithmId(sSecondAlgorithm);
        % skip comparisons against each other
        if(iFirstAlgorithmId == iSecondAlgorithmId)
            continue;
        end
%         disp([sFirstAlgorithm ' vs. ' sSecondAlgorithm]);
        % get winner and loser
        sChoice = char(ComparisonNode.item(c).getElementsByTagName('Choice').item(0).getFirstChild().getData());
        if(strcmp(sChoice, '0') == 1)
           sWinner = sFirstAlgorithm;
           sLoser = sSecondAlgorithm;
           Statistics.Total.WinnerFirst = Statistics.Total.WinnerFirst + 1;
        else
           sWinner = sSecondAlgorithm;
           sLoser = sFirstAlgorithm;
           Statistics.Total.WinnerSecond = Statistics.Total.WinnerSecond + 1;
        end
        iWinnerId = getAlgorithmId(sWinner);
        iLoserId = getAlgorithmId(sLoser);
        
        % same for first/second
        sModelName = getModelName(sFirst);
        iModelId = getModelId(sModelName);
        [ sLightDirection, sLightDistance ] = getLightPosition(sFirst);
        [ iLightDirectionId, iLightDistanceId ] = getLightPositionId(sLightDirection, sLightDistance);
        
        Statistics.Choices(i, c+1).sModelName = sModelName;
        Statistics.Choices(i, c+1).iModelId = iModelId;
        Statistics.Choices(i, c+1).sLightDirection = sLightDirection;
        Statistics.Choices(i, c+1).iLightDirectionId = iLightDirectionId;
        Statistics.Choices(i, c+1).sLightDistance = sLightDistance;
        Statistics.Choices(i, c+1).iLightDistanceId = iLightDistanceId;
        Statistics.Choices(i, c+1).sFirstAlgorithm = sFirstAlgorithm;
        Statistics.Choices(i, c+1).iFirstAlgorithm = getAlgorithmId(sFirstAlgorithm);
        Statistics.Choices(i, c+1).sSecondAlgorithm = sSecondAlgorithm;
        Statistics.Choices(i, c+1).iSecondAlgorithm = getAlgorithmId(sSecondAlgorithm);
        Statistics.Choices(i, c+1).sWinner = sWinner;
        Statistics.Choices(i, c+1).iWinnerId = iWinnerId;
        
        %% model
        Statistics.Model(iModelId).Wins(iWinnerId) = Statistics.Model(iModelId).Wins(iWinnerId) + 1;
        Statistics.Model(iModelId).Comparisons(iFirstAlgorithmId) = Statistics.Model(iModelId).Comparisons(iFirstAlgorithmId) + 1;
        Statistics.Model(iModelId).Comparisons(iSecondAlgorithmId) = Statistics.Model(iModelId).Comparisons(iSecondAlgorithmId) + 1;
        
        Statistics.Model(iModelId).WinsMatrix(iWinnerId, iLoserId) = Statistics.Model(iModelId).WinsMatrix(iWinnerId, iLoserId) + 1;
        
        Statistics.Model(iModelId).ComparisonsMatrix(iFirstAlgorithmId, iSecondAlgorithmId) = Statistics.Model(iModelId).ComparisonsMatrix(iFirstAlgorithmId, iSecondAlgorithmId) + 1;
        Statistics.Model(iModelId).ComparisonsMatrix(iSecondAlgorithmId, iFirstAlgorithmId) = Statistics.Model(iModelId).ComparisonsMatrix(iSecondAlgorithmId, iFirstAlgorithmId) + 1;
        
        %% light
        Statistics.Light(iLightDirectionId, iLightDistanceId).Wins(iWinnerId) = Statistics.Light(iLightDirectionId, iLightDistanceId).Wins(iWinnerId) + 1;
        Statistics.Light(iLightDirectionId, iLightDistanceId).Comparisons(iFirstAlgorithmId) = Statistics.Light(iLightDirectionId, iLightDistanceId).Comparisons(iFirstAlgorithmId) + 1;
        Statistics.Light(iLightDirectionId, iLightDistanceId).Comparisons(iSecondAlgorithmId) = Statistics.Light(iLightDirectionId, iLightDistanceId).Comparisons(iSecondAlgorithmId) + 1;
        
        Statistics.Light(iLightDirectionId, iLightDistanceId).WinsMatrix(iWinnerId, iLoserId) = Statistics.Light(iLightDirectionId, iLightDistanceId).WinsMatrix(iWinnerId, iLoserId) + 1;
        
        Statistics.Light(iLightDirectionId, iLightDistanceId).ComparisonsMatrix(iFirstAlgorithmId, iSecondAlgorithmId) = Statistics.Light(iLightDirectionId, iLightDistanceId).ComparisonsMatrix(iFirstAlgorithmId, iSecondAlgorithmId) + 1;
        Statistics.Light(iLightDirectionId, iLightDistanceId).ComparisonsMatrix(iSecondAlgorithmId, iFirstAlgorithmId) = Statistics.Light(iLightDirectionId, iLightDistanceId).ComparisonsMatrix(iSecondAlgorithmId, iFirstAlgorithmId) + 1;
        
        %% total
        Statistics.Total.Wins(iWinnerId) = Statistics.Total.Wins(iWinnerId) + 1;
        Statistics.Total.Comparisons(iFirstAlgorithmId) = Statistics.Total.Comparisons(iFirstAlgorithmId) + 1;
        Statistics.Total.Comparisons(iSecondAlgorithmId) = Statistics.Total.Comparisons(iSecondAlgorithmId) + 1;
        
        Statistics.Total.WinsMatrix(iWinnerId, iLoserId) = Statistics.Total.WinsMatrix(iWinnerId, iLoserId) + 1;
        
        Statistics.Total.ComparisonsMatrix(iFirstAlgorithmId, iSecondAlgorithmId) = Statistics.Total.ComparisonsMatrix(iFirstAlgorithmId, iSecondAlgorithmId) + 1;
        Statistics.Total.ComparisonsMatrix(iSecondAlgorithmId, iFirstAlgorithmId) = Statistics.Total.ComparisonsMatrix(iSecondAlgorithmId, iFirstAlgorithmId) + 1;
    end
end

for i=1:4
    Statistics.Total.WinsPercent(i) = 100 * Statistics.Total.Wins(i) / Statistics.Total.Comparisons(i);
    for j=1:4
        if(Statistics.Total.ComparisonsMatrix(i,j) == 0)
            continue;
        end
       Statistics.Total.WinsPercentMatrix(i,j) = 100 * Statistics.Total.WinsMatrix(i,j) / Statistics.Total.ComparisonsMatrix(i,j); 
    end
end


Statistics.Total.WinnerRatio = Statistics.Total.WinnerSecond / (Statistics.Total.WinnerFirst + Statistics.Total.WinnerSecond);

for k=1:3
    for i=1:4
        Statistics.Model(k).WinsPercent(i) = 100 * Statistics.Model(k).Wins(i) / Statistics.Model(k).Comparisons(i);
        for j=1:4
            if(Statistics.Model(k).ComparisonsMatrix(i,j) == 0)
                continue;
            end
            Statistics.Model(k).WinsPercentMatrix(i,j) = 100 * Statistics.Model(k).WinsMatrix(i,j) / Statistics.Model(k).ComparisonsMatrix(i,j);
        end
    end
end

for d=1:2
    for k=1:2
        for i=1:4
            Statistics.Light(k, d).WinsPercent(i) = 100 * Statistics.Light(k, d).Wins(i) / Statistics.Light(k, d).Comparisons(i);
            for j=1:4
                if(Statistics.Light(k, d).ComparisonsMatrix(i,j) == 0)
                    continue;
                end
                Statistics.Light(k, d).WinsPercentMatrix(i,j) = 100 * Statistics.Light(k, d).WinsMatrix(i,j) / Statistics.Light(k, d).ComparisonsMatrix(i,j);
            end
        end
    end
end

% real stats
for iAlgo=2:4
    for iModel=1:3
        for iLDir=1:2
            for iLDist=1:2
                [E, V, S, iChoices] = simpleStats(Statistics.Choices, iAlgo, iModel, iLDir, iLDist);
                i = 8*(iAlgo-2) + 4*(iModel-1) + 2*(iLDir-1) + iLDist;
                Statistics.Stats(1, i).E = E;
                Statistics.Stats(1, i).V = V;
                Statistics.Stats(1, i).S = S;
                Statistics.Stats(1, i).iChoices = iChoices;
                Statistics.Stats(1, i).iAlgorithmId = iAlgo;
                Statistics.Stats(1, i).sAlgorithmName = getAlgorithmName2(iAlgo);
                Statistics.Stats(1, i).iModelId = iModel;
                Statistics.Stats(1, i).sModelName = getModelName2(iModel);
                Statistics.Stats(1, i).iLightDirectionId = iLDir;
                Statistics.Stats(1, i).sLightDirectionName = getLightDirectionName2(iLDir);
                Statistics.Stats(1, i).iLightDistanceId = iLDist;
                Statistics.Stats(1, i).sLightDistanceName = getLightDistanceName2(iLDist);
            end
        end
    end
end
                
%% main function end
end


%% sub functions
function [ sAlgorithmName ] = getAlgorithmName( sImageName )
    iUnderlines = strfind(sImageName, '_');
    sAlgorithmName = sImageName(iUnderlines(length(iUnderlines))+1:length(sImageName)-4);
end

function [ sModelName ] = getModelName( sImageName )
    iSlashes = strfind(sImageName, '\');
    if isempty(iSlashes)
        iSlashes = strfind(sImageName, '/');
        iSlashes = iSlashes(length(iSlashes));
    end
    iUnderlines = strfind(sImageName, '_');
    sModelName = sImageName(iSlashes+1:iUnderlines(1)-1);
end

function [ iModelId ] = getModelId( sModelName )
    if strcmp(sModelName, 'Head') == 1
        iModelId = 1;
    end
    if strcmp(sModelName, 'HandOpen') == 1
        iModelId = 2;
    end
    if strcmp(sModelName, 'HandClosed') == 1
        iModelId = 3;
    end
end

function sModelName = getModelName2( iModelId )
    if iModelId == 1
        sModelName = 'Head';
    end
    if iModelId == 2
        sModelName = 'HandOpen';
    end
    if iModelId == 3
        sModelName = 'HandClosed';
    end
end

function [ sLightDirection, sLightDistance ] = getLightPosition( sImageName )
    iUnderlines = strfind(sImageName, '_');
    sLightDirection = sImageName(iUnderlines(2)+1:iUnderlines(3)-1);
    sLightDistance = sImageName(iUnderlines(3)+1:iUnderlines(4)-1);
end

function [ iLightDirectionId, iLightDistanceId ] = getLightPositionId(sLightDirection, sLightDistance)
    if strcmp(sLightDirection, 'side')
        iLightDirectionId = 1;
    end
    if strcmp(sLightDirection, 'behind')
        iLightDirectionId = 2;
    end
    
    if strcmp(sLightDistance, 'near')
        iLightDistanceId = 1;
    end
    if strcmp(sLightDistance, 'far')
        iLightDistanceId = 2;
    end
end

function sLightDirectionName = getLightDirectionName2( iLightDirectonId )
    if iLightDirectonId == 1
        sLightDirectionName = 'side';
    end
    if iLightDirectonId == 2
        sLightDirectionName = 'behind';
    end
end

function sLightDistanceName = getLightDistanceName2( iLightDistanceId )
    if iLightDistanceId == 1
        sLightDistanceName = 'near';
    end
    if iLightDistanceId == 2
        sLightDistanceName = 'far';
    end
end

function [ iSkillId ] = getSkillId( sSkill )
    if( strcmp(sSkill, 'consumer') == 1)
        iSkillId = 1;
    end
    if( strcmp(sSkill, 'basic') == 1)
        iSkillId = 2;
    end
    if( strcmp(sSkill, 'advanced') == 1)
        iSkillId = 3;
    end
    if( strcmp(sSkill, 'professional') == 1)
        iSkillId = 4;
    end
end

function [ iAlgorithmId ] = getAlgorithmId( sAlgorithmName )
    % jimenez at 1, because it is our reference algorithm
    if(strcmp(sAlgorithmName, 'Jimenez'))
        iAlgorithmId = 1;
    end
    if(strcmp(sAlgorithmName, 'dEon'))
        iAlgorithmId = 2;
    end
    if(strcmp(sAlgorithmName, 'Hable'))
        iAlgorithmId = 3;
    end
    if(strcmp(sAlgorithmName, 'Brand'))
        iAlgorithmId = 4;
    end
end

function [ sAlgorithmName ] = getAlgorithmName2( iAlgorithmId )
    if iAlgorithmId == 1
        sAlgorithmName = 'Jimenez';
    end
    if iAlgorithmId == 2
        sAlgorithmName = 'dEon';
    end
    if iAlgorithmId == 3
        sAlgorithmName = 'Hable';
    end
    if iAlgorithmId == 4
        sAlgorithmName = 'Brand';
    end
end

function [ Collector ] = createCollector()
    Collector.WinsMatrix = zeros(4, 4);
    Collector.WinsPercentMatrix = zeros(4, 4);
    Collector.ComparisonsMatrix = zeros(4, 4);
    Collector.Wins = zeros(1, 4);
    Collector.Comparisons = zeros(1, 4);
    Collector.WinsPercent = zeros(1, 4);
end

function [ E, V, S, iChoices ] = simpleStats( Choices, iAlgorithm, iModelId, iLightDirectionId, iLightDistanceId )
    [hChoices, wChoices] = size(Choices);
    c = 1;
    for h=1:hChoices
       for w=1:wChoices
           % skip values that do not match
           if ((Choices(h, w).iFirstAlgorithm ~= iAlgorithm) && (Choices(h, w).iSecondAlgorithm ~= iAlgorithm)) || (Choices(h, w).iModelId ~= iModelId) || (Choices(h, w).iLightDirectionId ~= iLightDirectionId) || (Choices(h, w).iLightDistanceId ~= iLightDistanceId)
               continue;
           end
           % always compare to reference algorithm!
           if Choices(h, w).iWinnerId == iAlgorithm
               iChoices(1, c) = 0; %#ok<AGROW>
           else
               iChoices(1, c) = 1; %#ok<AGROW>
           end
           c = c+1;
       end
    end
    
    E = mean(iChoices);
    V = var(iChoices);
    S = std(iChoices);
end
