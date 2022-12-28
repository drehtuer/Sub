function PlotFigures( Statistics )

for iModel=1:3
    for iLDir=1:2
        for iLDist=1:2
            chances = zeros(1,3);
            stds = zeros(1,3);
            for iAlgo=2:4
                i = 8*(iAlgo-2) + 4*(iModel-1) + 2*(iLDir-1) + iLDist;
                chances(iAlgo-1) = 1-Statistics.Stats(1, i).E;
                stds(iAlgo-1) = Statistics.Stats(1, i).S;
            end
            figure();
            hold on;
            ylabel('SSSSS Winning Ratio');
            
            title([Statistics.Stats(1, 1).sModelName ', light from ' Statistics.Stats(1, 1).sLightDirectionName ', camera distance ' Statistics.Stats(1, 1).sLightDistanceName]);
            bar(1:3, chances);
            errorbar(1:3, chances, stds);
            hold off;
        end
    end
end

end
