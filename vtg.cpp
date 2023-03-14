#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstddef>
#include <cctype>
#include <vector>

using namespace std;

string& trim(string& s);

int main(int argc, char *argv[])
{
	string temp;
	ifstream fin;
	ofstream fout;
	bool flag_out = false, flag_module = false, flag_dumpfile = false;
	bool flag_module_exist = false;
	char* fin_name = NULL, * fout_name = NULL;
	char* module_name = NULL;

	string module_name_read;
	string port_read, port_name;
	string out_str;

	size_t loc, j;
	unsigned int i;
	char c;

	vector<string> ports;

	while (--argc)
	{
		++argv;
		if (flag_out)
		{
			flag_out = false;
			fout_name = *argv;
		}
		else if (flag_module)
		{
			flag_module = false;
			module_name = *argv;
		}
		else if (strcmp(*argv, "-o") == 0)
			flag_out = true;
		else if (strcmp(*argv, "-m") == 0)
			flag_module = true;
		else if (strcmp(*argv, "-d") == 0)
			flag_dumpfile = true;
		else if (strcmp(*argv, "-h") == 0)
			/* show help info */;
		else
			fin_name = *argv;
	}
	if (fin_name != NULL && fout_name != NULL)
		fin.open(fin_name);
	else
	{
		cerr << "**Error: no input file or no output file" << endl;
		return -1;
	}
	if (!fin.is_open())
	{
		cerr << "**Error: cannot open \'" << fin_name << '\'' << endl;
		return 1;
	}
	while (flag_module_exist == false && fin >> temp)
	{
		if (temp == "`timescale" || temp == "`define")
		{
			out_str += temp;
			getline(fin, temp);
			out_str += temp + '\n';
		}
		else if (temp == "module")
		{
			module_name_read.clear();
			while (isspace(c = fin.get()))
				;
			while (isalnum(c) || c == '_')
			{	/* read module name */
				module_name_read += c;
				c = fin.get();
			}
			if (module_name != NULL && module_name_read != module_name)
				module_name_read.clear();
			else
			{	/* target module found */
				flag_module_exist = true;
				while (c != '(' && c != EOF)
					c = fin.get();
				while (isspace(c = fin.get()) && c != EOF)
					;
				/* begin to read ports */
				while (c != EOF)
				{
					if (c != ',' && c != ')')
					{
						port_read += c;
						c = fin.get();
					}
					else
					{
						/* erase space in the head and tail of port_read */
						trim(port_read);
						ports.push_back(port_read);
						port_read.clear();
						if (c == ')')
							break;
						while (isspace(c = fin.get()))
							;
					}
				}
				if (c == EOF)
				{
					fin.close();
					cerr << "**Error: cannot read ports of module \'" << module_name_read << '\'' << endl;
					return 2;
				}
			}
			/* search for "endmodule" */
			while (getline(fin, temp))
				if (temp.find("endmodule") != string::npos)
					break;
		}
	}
	fin.close();
	if (!flag_module_exist)
	{
		if (module_name != NULL)
			cerr << "**Error: module \'" << module_name << "\' does not exist" << endl;
		else
			cerr << "**Error: no module in file \'" << fin_name << '\'' << endl;
		return 3;
	}
	/* prepare for output */
	out_str += "\nmodule " + module_name_read + "_test_g();\n";
	for (i = 0; i < ports.size(); ++i)
	{
		if ((loc = ports.at(i).find("input")) != string::npos)
			ports.at(i).replace(loc, 5, "reg");
		else if ((loc = ports.at(i).find("output")) != string::npos)
			ports.at(i).replace(loc, 6, "wire");
		out_str += '\t' + ports.at(i) + ";\n";
	}
	out_str += "\n\t" + module_name_read + ' ' + module_name_read + "_test_inst(";
	/* get the name of ports */
	for (i = 0; i < ports.size(); ++i)
	{
		port_name.clear();
		for (j = ports.at(i).size(); j > 0 && (isalnum(c = ports.at(i).at(j - 1)) || c == '_'); --j)
			port_name = c + port_name;
		if (i < ports.size() - 1)
			out_str += port_name + ", ";
		else
			out_str += port_name + ");\n\n\n";
	}
	out_str += "\tinitial begin\n\t\t$dumpfile(\"";
	out_str += module_name_read + ".vcd\");\n\t\t$dumpvars(0, " + module_name_read + "_test_g);\n\tend\n";
	out_str += "endmodule";

	fout.open(fout_name);
	if(!fout.is_open())
	{
		cerr << "**Error: cannot open \'" << fout_name << '\'' << endl;
		return 1;
	}
	fout << out_str << endl;
	fout.close();
	return 0;
}

/* clear white space */
string& trim(string& s)
{
	size_t i;

	for (i = 0; i < s.size(); ++i)
		if (!isspace(s.at(i)))
			break;
	s.erase(0, i);
	if (!s.empty())
		for (i = s.size() - 1; i > 0; --i)
			if (!isspace(s.at(i)))
				break;
	s.erase(i + 1);
	return s;
}