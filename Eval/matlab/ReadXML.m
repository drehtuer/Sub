function [ Data, Comparisons ] = ReadXML( sFilename, iReferenceAlgorithmId )
%READXML Summary of this function goes here
%   Detailed explanation goes here

    %% read xml
    RootNode = xmlread(sFilename);
    ResultsNode = RootNode.getElementsByTagName('Results');
    % id
    IdNode = ResultsNode.item(0).getElementsByTagName('Id');
    Data.sKey = char(IdNode.item(0).getAttribute('key'));
    disp(['Id: ' Data.sKey]);
    % questionaire
    QuestionnaireNode = ResultsNode.item(0).getElementsByTagName('Questionaire');
    AnswerNode = QuestionnaireNode.item(0).getElementsByTagName('Answer');
    
    Data.iAge = str2num(char(AnswerNode.item(0).getFirstChild().getData())); %#ok<ST2NM>
    Data.sSex = char(AnswerNode.item(1).getFirstChild().getData());
    if(strcmp(Data.sSex, 'm') == 1)
        Data.iSex = 0;
    else
        Data.iSex = 1;
    end
    Data.sSkill = char(AnswerNode.item(2).getFirstChild().getData());
    Data.iSkill = getSkillId(Data.sSkill);
    
    disp(['Age ' num2str(Data.iAge) ', Sex: ' Data.sSex ', Skill: ' Data.sSkill]);
    
    %% comparisons
    ComparisonNode = ResultsNode.item(0).getElementsByTagName('Comparison');
    for c=0:ComparisonNode.getLength()-1
        cc = c+1;
        % for all comparisons
        sFirst = char(ComparisonNode.item(c).getElementsByTagName('First').item(0).getFirstChild().getData());
        Comparisons(cc).sFirstAlgorithm = getAlgorithmName(sFirst);
        Comparisons(cc).iFirstAlgorithmId = getAlgorithmId(Comparisons(cc).sFirstAlgorithm);
        sSecond = char(ComparisonNode.item(c).getElementsByTagName('Second').item(0).getFirstChild().getData());
        Comparisons(cc).sSecondAlgorithm = getAlgorithmName(sSecond);
        Comparisons(cc).iSecondAlgorithmId = getAlgorithmId(Comparisons(cc).sSecondAlgorithm);
        % skip comparisons against each other
        if(Comparisons(cc).iFirstAlgorithmId == Comparisons(cc).iSecondAlgorithmId)
            continue;
        end
        sChoice = char(ComparisonNode.item(c).getElementsByTagName('Choice').item(0).getFirstChild().getData());
        if(strcmp(sChoice, '0') == 1)
            Comparisons(cc).sWinner = Comparisons(cc).sFirstAlgorithm;
            Comparisons(cc).sLoser = Comparisons(cc).sSecondAlgorithm;
        else
            Comparisons(cc).sWinner = Comparisons(cc).sSecondAlgorithm;
            Comparisons(cc).sLoser = Comparisons(cc).sFirstAlgorithm;
        end
        Comparisons(cc).iWinnerId = getAlgorithmId(Comparisons(cc).sWinner);
        Comparisons(cc).iLoserId = getAlgorithmId(Comparisons(cc).sLoser);
        if iReferenceAlgorithmId == Comparisons(cc).iWinnerId
            Comparisons(cc).iChoice = 0;
        else
            Comparisons(cc).iChoice = 1;
        end
        
        % same for first/second
        Comparisons(cc).sModelName = getModelName(sFirst);
        Comparisons(cc).iModelId = getModelId(Comparisons(cc).sModelName);
        
        [ Comparisons(cc).sLightDirectionName, Comparisons(cc).sLightDistanceName ] = getLightPosition(sFirst);
        [ Comparisons(cc).iLightDirectionId, Comparisons(cc).iLightDistanceId ] = getLightPositionId(Comparisons(cc).sLightDirectionName, Comparisons(cc).sLightDistanceName);
    end
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

