//
// Created by fury on 8/4/15.
//

#ifndef G09DNNTSBUILDER_READERS_H
#define G09DNNTSBUILDER_READERS_H

string forceFinder(const string &filename)
{
    string force_csv;
    regex pattern_force("Hartrees/Bohr");
    regex pattern_cart("Cartesian");
    regex pattern_value("\-?[[:d:]]+\.[[:d:]]+");
    string line;
    while (getline(inFile, filename))
    {
        if (regex_search(line, pattern_force))
        {
            string line2;
            while (getline(inFile, line2) && !regex_search(line2, pattern_cart))
            {
                sregex_iterator pos(line2.begin(), line2.end(), pattern_value);
                sregex_iterator end;
                for (; pos != end; ++pos)
                {
                    force_csv += pos->str(0) + ",";
                }
            }
        }
    }
    return force_csv;
}


#endif //G09DNNTSBUILDER_READERS_H
