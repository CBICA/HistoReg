#include <iostream>
#include <fstream>
//#include <bits/stdc++.h> 
//#include <sys/stat.h> 
// #include <sys/types.h> 
//#include <string>
// #include <stdlib.h>

#ifdef WIN32
	#include <direct.h>
#endif

//#include <sys/dir.h>
#include <chrono>

#include "itkImageIOBase.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

//#include "../greedy/src/GreedyAPI.h"




using namespace std;

bool BothAreSpaces(char lhs, char rhs) { return (lhs == rhs) && (lhs == ' '); }

string basename(string const& input)
{
#ifdef _WIN32
	string cutted_slash = input.substr(0, input.size() - (( '\\' == input[input.size() - 1]) ? 1 : 0));
	size_t slash_pos = cutted_slash.find_last_of( '\\' );
	return cutted_slash.substr(string::npos == slash_pos ? 0 : slash_pos + 1);
#else
	string cutted_slash = input.substr(0, input.size() - (('/' == input[input.size() - 1]) ? 1 : 0));
	size_t slash_pos = cutted_slash.find_last_of('/');
	return cutted_slash.substr(string::npos == slash_pos ? 0 : slash_pos + 1);
#endif // _WIN32
}

bool createDir(const std::string &dir_name)
{
	//! Pure c++ based directory creation
#if defined(_WIN32)
	DWORD ftyp = GetFileAttributesA(dir_name.c_str()); // check if directory exists or not
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		_mkdir(dir_name.c_str());
	return true;
#else
	DIR *pDir;
	pDir = opendir(dir_name.c_str()); // check if directory exists or not
	if (pDir == NULL)
		mkdir(dir_name.c_str(), 0777);
	return true;
#endif
	return false;
}


string GetStdoutFromCommand(string cmd) 
{
    string data;
    FILE * stream;
    const int max_buffer = 256;
    char buffer[max_buffer];
    cmd.append(" 2>&1");

	#ifdef WIN32
		stream = _popen(cmd.c_str(), "r");
		if (stream) {
		while (!feof(stream))
		if (fgets(buffer, max_buffer, stream) != NULL) data.append(buffer);
		_pclose(stream);
		}
		return data;
	#else
		stream = popen(cmd.c_str(), "r");
		if (stream) {
			while (!feof(stream))
				if (fgets(buffer, max_buffer, stream) != NULL) data.append(buffer);
			pclose(stream);
	}
		return data;
	#endif
}

static void show_usage(string name)
{
    cerr << "Usage: " << name << " <option(s)>"
              << "Options:\n"
              << "\t-h,--help\t\tShow this help message\n"
              << '\n'
              << "Compulsory arguments:\n"
              << "\t-m,--moving [PATH to file]\tPATH to the moving image\n"
              << "\t-f,--fixed [PATH to file]\tPATH to the fixed image\n"
              << "\t-c,--c2d_executable [PATH to file]\tPath to c2d executable\n"
              << "\t-g,--greedy_executable [PATH to file]\tPath to greedy executable\n"
              << "\t-o,--output [PATH to folder]\tSpecify the destination path\n"
              << '\n'
              << "Optional arguments:\n"
              << "\t-l,--landmarks [PATH to file]\tPath to the source landmarks, apply registration to these landmarks.\n\t\tMust be a csv file with x,y values starting 2nd row, 2nd line."
              << "\t-F,--FullResolution\tApply registration to the full resolution RGB images\n"
              << "\t-r,--resample [VALUE]\tPercentage of the full resolution the images will be resampled to, used for computation.\n\t\tMust be a percentage but DO NOT add %. Default: 4%\n"
              << "\t-i,--iteration [VALUE]\tNumber of iteration used for initial brute search before affine registration.\n\t\tMust be a integer. Default: 5000.\n"
              << "\t-k,--kernel [VALUE]\tDefine size of the kernel, it will be the size of the resampled image divided by this value.\n\t\tMust be an integer. Small values will increase size of the kernel and so increase runtime.\n"
              << endl;
}

int main(int argc, char* argv[])
{
    chrono::time_point<chrono::system_clock> start_script, end_script;
    int duration;
    start_script = chrono::system_clock::now();

    cout << "Starting..." << '\n';


    if (argc < 4) { // We expect 4 arguments: the program name, the source path, the target path and the output path
        show_usage(argv[0]);
        return 1;
    }

    // PATH
    string PATH_Output_DIR;
    string PATH_source;
    string PATH_target;
    string PATH_landmarks;
    string PATH_Output_Temp;

    // Optional arguments
    string resample = "4";
    string Kernel_Divider = "40";
    string iteration = "5000";

    // executables
    string c2d_executable;
    string greedy_executable;
    
    // Flags
    int Output_provided = 0;
    int Fixed_provided = 0;
    int Moving_provided = 0;
    int c2d_executable_provided = 0;
	int greedy_executable_provided = 0;
    int Flag_Full_Resolution = 0;
    int Flag_landmarks = 0;
    int PATH_Output_Temp_provided = 0;

    cout << "Reading arguments..." << '\n';

    for (int i=1; i<argc; i++){
        string arg = argv[i];
        if ((arg == "-h") || (arg == "--help")){
            show_usage(argv[0]);
            return 1;
        } else {
            if ((arg == "-o") || (arg == "--output")){
                if (i + 1 < argc){ // Make sure we aren't at the end of argv!
                    PATH_Output_DIR = argv[++i]; // Increment 'i' so we don't get the argument as the next argv[i].
                    Output_provided = 1;
                } else { // Uh-oh, there was no argument to the destination option.
                    cerr << "--output option requires one argument." << '\n';
                    return 1;
                }
            }
            if ((arg == "-m") || (arg == "--moving")){
                if (i + 1 < argc){ // Make sure we aren't at the end of argv!
                    PATH_source = argv[++i]; // Increment 'i' so we don't get the argument as the next argv[i].
                    Moving_provided = 1;
                } else { // Uh-oh, there was no argument to the destination option.
                    cerr << "--moving option requires one argument." << '\n';
                    return 1;
                }
            }
            if ((arg == "-f") || (arg == "--fixed")){
                if (i + 1 < argc){ // Make sure we aren't at the end of argv!
                    PATH_target = argv[++i]; // Increment 'i' so we don't get the argument as the next argv[i].
                    Fixed_provided = 1;
                } else { // Uh-oh, there was no argument to the destination option.
                    cerr << "--target option requires one argument." << '\n';
                    return 1;
                }
            }
            if ((arg == "-F") || (arg == "--FullResolution")){
                Flag_Full_Resolution = 1;
            }
            if ((arg == "-r") || (arg == "--resample")){
                if (i + 1 < argc){ // Make sure we aren't at the end of argv!
                    resample = argv[++i];
                }else { // Uh-oh, there was no argument to the destination option.
                    cerr << "--resample option requires one argument." << '\n';
                    return 1;
                }
            }
            if ((arg == "-k") || (arg == "--kernel")){
                if (i + 1 < argc){ // Make sure we aren't at the end of argv!
                    Kernel_Divider = argv[++i];
                }else { // Uh-oh, there was no argument to the destination option.
                    cerr << "--kernel option requires one argument." << '\n';
                    return 1;
                }
            }
            if ((arg == "-i") || (arg == "--iteration")){
                if (i + 1 < argc){ // Make sure we aren't at the end of argv!
                    iteration = argv[++i];
                }else { // Uh-oh, there was no argument to the destination option.
                    cerr << "--iteration option requires one argument." << '\n';
                    return 1;
                }
            }
            if ((arg == "-l") || (arg == "--landmarks")){
                if (i + 1 < argc){ // Make sure we aren't at the end of argv!
                    PATH_landmarks = argv[++i];
                    Flag_landmarks = 1;
                }else { // Uh-oh, there was no argument to the destination option.
                    cerr << "--landmarks option requires one argument." << '\n';
                    return 1;
                }
            }
            if ((arg == "-c") || (arg == "--c2d_executable")){
                if (i + 1 < argc){ // Make sure we aren't at the end of argv!
                    c2d_executable = argv[++i];
                    c2d_executable_provided = 1;
                }else { // Uh-oh, there was no argument to the destination option.
                    cerr << "--c2d_executable option requires one argument." << '\n';
                    return 1;
                }
            }
			if ((arg == "-g") || (arg == "--greedy_executable")) {
				if (i + 1 < argc) { // Make sure we aren't at the end of argv!
					greedy_executable = argv[++i];
					greedy_executable_provided = 1;
				}
				else { // Uh-oh, there was no argument to the destination option.
					cerr << "--greedy_executable option requires one argument." << '\n';
					return 1;
				}
			}
            if ((arg == "-t") || (arg == "--tmp_directory")) {
				if (i + 1 < argc) { // Make sure we aren't at the end of argv!
					PATH_Output_Temp = argv[++i];
					PATH_Output_Temp_provided = 1;
				}
				else { // Uh-oh, there was no argument to the destination option.
					cerr << "--tmp_directory option requires one argument." << '\n';
					return 1;
				}
			}
        }
    }

    // Check compulsory arguments
    if ((Output_provided == 0) || (Moving_provided == 0) || (Fixed_provided == 0) || (c2d_executable_provided == 0) || (greedy_executable_provided == 0)){
        cerr << "Error: Missing compulsory argument : -m, -f, -o, -c, -g" << '\n';
        return 1;
    }

    string s1 = "6";
    string s2 = "5";

    cout << "Done." << '\n';
    
    // Prints arguments
    
    // cout << PATH_source << '\n';
    // cout << PATH_target << '\n';
    // cout << PATH_Output_DIR << '\n';

    cout << "Creating output and temporary directories..." << '\n';

    // Get name images for output folder
    string name_source_full = basename (PATH_source.c_str());
    string name_target_full = basename (PATH_target.c_str());
    string delimiter = ".";

    string name_source = name_source_full.substr(0,name_source_full.find(delimiter));
    string name_target = name_target_full.substr(0,name_target_full.find(delimiter));

    // Create string for output folder
    string PATH_Output = PATH_Output_DIR + string("/") + name_source + string("_to_") + name_target;
    
    if ( PATH_Output_Temp_provided == 0){
        PATH_Output_Temp = PATH_Output + "/tmp";
    }

    // Create output folder
	createDir(PATH_Output_DIR.c_str());
    createDir(PATH_Output.c_str());
    createDir(PATH_Output_Temp.c_str());
	
    cout << "Done." << '\n';

    // Extract sizes
    // Target
    string Size_original_target_W;
    string Size_original_target_H;
    itk::ImageIOBase::Pointer m_itkImageIOBase = itk::ImageIOFactory::CreateImageIO(PATH_target.c_str(), itk::ImageIOFactory::ReadMode);
    m_itkImageIOBase->SetFileName(PATH_target);
    m_itkImageIOBase->ReadImageInformation();

    if ( m_itkImageIOBase->GetNumberOfDimensions() > 2 ){
        cerr << "Error : input image has more than 2 dimensions." << '\n';
    }
    else {
        Size_original_target_W = to_string(m_itkImageIOBase->GetDimensions(0));
        Size_original_target_H = to_string(m_itkImageIOBase->GetDimensions(1));
    }

    // Source
    string Size_original_source_W;
    string Size_original_source_H;
    m_itkImageIOBase = itk::ImageIOFactory::CreateImageIO(PATH_source.c_str(), itk::ImageIOFactory::ReadMode);
    m_itkImageIOBase->SetFileName(PATH_source);
    m_itkImageIOBase->ReadImageInformation();

    if ( m_itkImageIOBase->GetNumberOfDimensions() > 2 ){
        cerr << "Error : input image has more than 2 dimensions." << '\n';
    }
    else {
        Size_original_source_W = to_string(m_itkImageIOBase->GetDimensions(0));
        Size_original_source_H = to_string(m_itkImageIOBase->GetDimensions(1));
    }

    // Print sizes
    // cout << "   Target_W : " << Size_original_target_W << '\n';
    // cout << "   Target_H : " << Size_original_target_H << '\n';
    // cout << "   Source_W : " << Size_original_source_W << '\n';
    // cout << "   Source_H : " << Size_original_source_H << '\n';

    // Preprocessing
    cout << "Preprocessing..." << '\n';

    string Resampling_command = "-smooth-fast 6x5vox -resample " + resample + "% -spacing 1x1mm -orient LP -origin 0x0mm -o";

    string PATH_small_target = PATH_Output + string("/new_small_target.nii.gz");
    string PATH_small_source = PATH_Output + string("/new_small_source.nii.gz");

    string command_source = c2d_executable + string(" ") + PATH_target + string(" ") + Resampling_command + string(" ") + PATH_small_target;
    string command_target = c2d_executable + string(" ") + PATH_source + string(" ") + Resampling_command + string(" ") + PATH_small_source;

    cout << "   Resampling source..." << '\n';
    system(command_source.c_str());
    cout << "   Resampling target..." << '\n';
    system(command_target.c_str());

    cout << "   Padding images..." << '\n';

    // Extract sizes
    // Target
    string Size_small_target_W;
    string Size_small_target_H;
    m_itkImageIOBase = itk::ImageIOFactory::CreateImageIO(PATH_small_target.c_str(), itk::ImageIOFactory::ReadMode);
    m_itkImageIOBase->SetFileName(PATH_small_target);
    m_itkImageIOBase->ReadImageInformation();

    if ( m_itkImageIOBase->GetNumberOfDimensions() > 2 ){
        cerr << "Error : input image has more than 2 dimensions." << '\n';
    }
    else {
        Size_small_target_W = to_string(m_itkImageIOBase->GetDimensions(0));
        Size_small_target_H = to_string(m_itkImageIOBase->GetDimensions(1));
    }

    // Source
    string Size_small_source_W;
    string Size_small_source_H;
    m_itkImageIOBase = itk::ImageIOFactory::CreateImageIO(PATH_small_source.c_str(), itk::ImageIOFactory::ReadMode);
    m_itkImageIOBase->SetFileName(PATH_small_source);
    m_itkImageIOBase->ReadImageInformation();

    if ( m_itkImageIOBase->GetNumberOfDimensions() > 2 ){
        cerr << "Error : input image has more than 2 dimensions." << '\n';
    }
    else {
        Size_small_source_W = to_string(m_itkImageIOBase->GetDimensions(0));
        Size_small_source_H = to_string(m_itkImageIOBase->GetDimensions(1));
    }
    
    // Print sizes
    // cout << "   Small_target_W : " << Size_small_target_W << '\n';
    // cout << "   Small_target_H : " << Size_small_target_H << '\n';
    // cout << "   Small_source_W : " << Size_small_source_W << '\n';
    // cout << "   Small_source_H : " << Size_small_source_H << '\n';
    
    // Get intensities from 4 cornees
    // Computes size kernel
    int kernel_W = stoi(Size_small_source_W) / stoi(Kernel_Divider);
    int kernel_H = stoi(Size_small_source_H) / stoi(Kernel_Divider);

    // Makes the kernel a square
    int kernel = kernel_H; 
    if ( kernel_H < kernel_W )
    {
        kernel = kernel_W;
    }

    // Create the command to get the intensites of the corners
    int Size_W_minus_kernel_target = stoi(Size_small_target_W) - kernel;
    int Size_H_minus_kernel_target = stoi(Size_small_target_H) - kernel;

	string grep;
#ifdef WIN32
	grep = " | findstr /C:\" 1 \"";
#else
	grep = " | grep \" 1 \"";
#endif

	string Four_corners_command = c2d_executable + string(" ") + PATH_small_target + string(" -dup -cmv -popas Y -popas X -push X -thresh 0 ") + to_string(kernel) + string(" 1 0 -push Y -thresh 0 ") + to_string(kernel) + string(" 1 0 -times -popas c00 -push X -thresh ") + to_string(Size_W_minus_kernel_target) + string(" ") + Size_small_target_W + string(" 1 0 -push Y -thresh 0 ") + to_string(kernel) + string(" 1 0 -times -popas c01 -push X -thresh ") + to_string(Size_W_minus_kernel_target) + string(" ") + Size_small_target_W + string(" 1 0 -push Y -thresh ") + to_string(Size_H_minus_kernel_target) + string(" ") + Size_small_target_H + string(" 1 0 -times -popas c11 -push X -thresh 0 ") + to_string(kernel) + string(" 1 0 -push Y -thresh ") + to_string(Size_H_minus_kernel_target) + string(" ") + Size_small_target_H + string(" 1 0 -times -popas c10 -push c00 -push c01 -push c11 -push c10 -add -add -add -lstat" + grep);

    string stats_target = GetStdoutFromCommand(Four_corners_command);

    // Replace multiple consecutive space in the string by only one
    string::iterator new_end = unique(stats_target.begin(), stats_target.end(), BothAreSpaces);
    stats_target.erase(new_end, stats_target.end());

    //Extract meand and std
    delimiter = " ";
    stats_target.erase(0,stats_target.find(delimiter) + delimiter.length());
    stats_target.erase(0,stats_target.find(delimiter) + delimiter.length());
    string mean_target = stats_target.substr(0,stats_target.find(delimiter));
    stats_target.erase(0,stats_target.find(delimiter) + delimiter.length());
    string std_target = stats_target.substr(0,stats_target.find(delimiter));

    // Same with source
    int Size_W_minus_kernel_source = stoi(Size_small_source_W) - kernel;
    int Size_H_minus_kernel_source = stoi(Size_small_source_H) - kernel;

    Four_corners_command = c2d_executable + string(" ") + PATH_small_source + string(" -dup -cmv -popas Y -popas X -push X -thresh 0 ") + to_string(kernel) + string(" 1 0 -push Y -thresh 0 ") + to_string(kernel) + string(" 1 0 -times -popas c00 -push X -thresh ") + to_string(Size_W_minus_kernel_source) + string(" ") + Size_small_source_W + string(" 1 0 -push Y -thresh 0 ") + to_string(kernel) + string(" 1 0 -times -popas c01 -push X -thresh ") + to_string(Size_W_minus_kernel_source) + string(" ") + Size_small_source_W + string(" 1 0 -push Y -thresh ") + to_string(Size_H_minus_kernel_source) + string(" ") + Size_small_source_H + string(" 1 0 -times -popas c11 -push X -thresh 0 ") + to_string(kernel) + string(" 1 0 -push Y -thresh ") + to_string(Size_H_minus_kernel_source) + string(" ") + Size_small_source_H + string(" 1 0 -times -popas c10 -push c00 -push c01 -push c11 -push c10 -add -add -add -lstat" + grep);
	
	string stats_source = GetStdoutFromCommand(Four_corners_command);

    // Replace multiple consecutive space by only one
    new_end = unique(stats_source.begin(), stats_source.end(), BothAreSpaces);
    stats_source.erase(new_end, stats_source.end());

    //Extract meand and std
    delimiter = " ";
    stats_source.erase(0,stats_source.find(delimiter) + delimiter.length());
    stats_source.erase(0,stats_source.find(delimiter) + delimiter.length());
    string mean_source = stats_source.substr(0,stats_source.find(delimiter));
    stats_source.erase(0,stats_source.find(delimiter) + delimiter.length());
    string std_source = stats_source.substr(0,stats_source.find(delimiter));

    // Compute four kernel
    int four_kernel = 4*kernel;

    string New_size_H = Size_small_target_H;
    string New_size_W = Size_small_target_W;

    string PATH_small_target_padded = PATH_Output + string("/new_small_target_padded.nii.gz");
    string PATH_small_source_padded = PATH_Output + string("/new_small_source_padded.nii.gz");

    if ( Size_small_source_H != Size_small_target_H || Size_small_source_W != Size_small_target_W )
    {
        //cout << "   Modifying sizes..." << '\n';
        if ( Size_small_target_W < Size_small_source_W )
        {
            New_size_W = Size_small_source_W;
        }
        if ( Size_small_target_H < Size_small_source_H )
        {
            New_size_H = Size_small_source_H;
        }

        //cout << "   New size is : " << New_size_W << "x" << New_size_H << '\n';

        string command_resize_target = c2d_executable + string(" ") + PATH_small_target + string(" -pad-to ") + New_size_W + string("x") + New_size_H + string(" 0 -o ") + PATH_small_target_padded;
        string command_resize_source = c2d_executable + string(" ") + PATH_small_source + string(" -pad-to ") + New_size_W + string("x") + New_size_H + string(" 0 -o ") + PATH_small_source_padded;
      
        //cout << "   Resizing target..." << '\n';
        system(command_resize_target.c_str());
        //cout << "   Resizing source..." << '\n';
        system(command_resize_source.c_str());
    }
    else
    {
        // !!! DOESN'T WORK IF I NEED ORIGINAL IMAGES BEFORE PADDING
        PATH_small_target_padded = PATH_small_target;
        PATH_small_source_padded = PATH_small_source;
    }

    // Pad with good intensity
    // Target
    string PATH_mask_target = PATH_Output_Temp + string("/mask_target.nii.gz");
    string command = c2d_executable + string(" ") + PATH_small_target_padded + string(" -thresh 1 inf 1 0 -pad ") + to_string(four_kernel) + string("x") + to_string(four_kernel) + string(" ") + to_string(four_kernel) + string("x") + to_string(four_kernel) + string(" 0 -o ") + PATH_mask_target;
    system(command.c_str());

    command = c2d_executable + string(" ") + PATH_mask_target + string(" -replace 0 1 1 0 -popas invmask ") + PATH_mask_target + string(" -replace 0 1 1 1 -scale ") + mean_target + string(" -noise-gaussian ") + std_target + string(" -push invmask -times -o ") + PATH_mask_target;
    system(command.c_str());

    command = c2d_executable + string(" ") + PATH_small_target_padded + string(" -pad ") + to_string(four_kernel) + string("x") + to_string(four_kernel) + string(" ") + to_string(four_kernel) + string("x") + to_string(four_kernel) + string(" 0 -o ") + PATH_small_target_padded;
    system(command.c_str());

    command = c2d_executable + string(" ") + PATH_mask_target + string(" ") + PATH_small_target_padded + string(" -add -o ") + PATH_small_target_padded;
    system(command.c_str());

    cout << "   Target done." << '\n';


    // Source
    string PATH_mask_source = PATH_Output_Temp + string("/mask_source.nii.gz");
    command = c2d_executable + string(" ") + PATH_small_source_padded + string(" -thresh 1 inf 1 0 -pad ") + to_string(four_kernel) + string("x") + to_string(four_kernel) + string(" ") + to_string(four_kernel) + string("x") + to_string(four_kernel) + string(" 0 -o ") + PATH_mask_source;
    system(command.c_str());

    command = c2d_executable + " " + PATH_mask_source + string(" -replace 0 1 1 0 -popas invmask ") + PATH_mask_source + string(" -replace 0 1 1 1 -scale ") + mean_source + string(" -noise-gaussian ") + std_source + string(" -push invmask -times -o ") + PATH_mask_source;
    system(command.c_str());

    command = c2d_executable + " " + PATH_small_source_padded + string(" -pad ") + to_string(four_kernel) + string("x") + to_string(four_kernel) + string(" ") + to_string(four_kernel) + string("x") + to_string(four_kernel) + string(" 0 -o ") + PATH_small_source_padded;
    system(command.c_str());

    command = c2d_executable + " " + PATH_mask_source + string(" ") + PATH_small_source_padded + string(" -add -o ") + PATH_small_source_padded;
    system(command.c_str());

    cout << "   Source done." << '\n';

    // Extract sizes
    // Target
    string Size_small_target_padded_W;
    string Size_small_target_padded_H;
    m_itkImageIOBase = itk::ImageIOFactory::CreateImageIO(PATH_small_target_padded.c_str(), itk::ImageIOFactory::ReadMode);
    m_itkImageIOBase->SetFileName(PATH_small_target_padded);
    m_itkImageIOBase->ReadImageInformation();

    if ( m_itkImageIOBase->GetNumberOfDimensions() > 2 ){
        cerr << "Error : input image has more than 2 dimensions." << '\n';
    }
    else {
        Size_small_target_padded_W = to_string(m_itkImageIOBase->GetDimensions(0));
        Size_small_target_padded_H = to_string(m_itkImageIOBase->GetDimensions(1));
    }

    // // Print sizes
    // cout << "   Small_target_W : " << Size_small_target_padded_W << '\n';
    // cout << "   Small_target_H : " << Size_small_target_padded_H << '\n';

    cout << "Registration..." << '\n';
    cout << "   Computing affine..." << '\n';

    // Registration
    // Affine
    int offset = (stoi(Size_small_target_H) + four_kernel) / 10;
    string PATH_small_affine = PATH_Output + string("/small_Affine.mat");
    string command_affine = greedy_executable + " -d 2 -a -search " + iteration + " 180 " + to_string(offset) + " -m NCC " + to_string(kernel) + "x" + to_string(kernel) + " -i " + PATH_small_target_padded + " " + PATH_small_source_padded + " -o " + PATH_small_affine + " -gm-trim " + to_string(kernel) + "x" + to_string(kernel) + " -n 150x50x10 -ia-image-centers";
    system(command_affine.c_str());

    cout << "   Computing Defformable..." << '\n';
    // Deformable
    string PATH_small_warp = PATH_Output + string("/small_warp.nii.gz");
    string PATH_small_inv_warp = PATH_Output + string("/small_inv_warp.nii.gz");
    string command_defformable = greedy_executable + " -d 2 -m NCC " + to_string(kernel) + "x" + to_string(kernel) + " -i " + PATH_small_target_padded + " " + PATH_small_source_padded + " -it " + PATH_small_affine + " -o " + PATH_small_warp + " -oinv " + PATH_small_inv_warp + " -n 150x50x10 -s " + s1 + "vox" + " " + s2 + "vox"; 
    system(command_defformable.c_str());
    
    cout << "   Applying registration to small grayscale images..." << '\n';
    // Apply to small images
    string PATH_small_registered_image = PATH_Output + string("/small_registeredImage.nii.gz");
    string command_reslice_small = greedy_executable + " -d 2 -rf " + PATH_small_target_padded + " -rm " + PATH_small_source_padded + " " + PATH_small_registered_image + " -r " + PATH_small_warp + " " + PATH_small_affine;
    system(command_reslice_small.c_str());
    
    cout << "   Adaptating registration mectrics to non-padded full resolution RGB images..." << '\n';
    // Compute metrics for full resolution images
    // Affine
    ifstream smallAffFile;
    string STRING;
    int factor = 100/stoi(resample);
    long double my_var[9];
    string test;
    delimiter = " ";
    int i = 0;

    // Read small affine
    smallAffFile.open (PATH_small_affine);
    while(!smallAffFile.eof())
    {
        getline(smallAffFile,STRING);

        stringstream(STRING.substr(0,STRING.find(delimiter))) >> my_var[i];
        STRING.erase(0,STRING.find(delimiter) + delimiter.length());
        i++;

        stringstream(STRING.substr(0,STRING.find(delimiter))) >> my_var[i];
        STRING.erase(0,STRING.find(delimiter) + delimiter.length());
        i++;

        stringstream(STRING.substr(0,STRING.find(delimiter))) >> my_var[i];
        STRING.erase(0,STRING.find(delimiter) + delimiter.length());
        i++;
    }
    smallAffFile.close();

    // Modify translation vector
    long double new_val_1 = my_var[2] * factor;
    long double new_val_2 = my_var[5] * factor;

    // Write new matrix adapted to full resolution images
    string PATH_affine = PATH_Output + "/Affine.mat";
    ofstream AffineFile;
    AffineFile.open(PATH_affine);
    AffineFile << to_string(my_var[0]) + " " + to_string(my_var[1]) + " " + to_string(new_val_1) + '\n' + to_string(my_var[3]) + " " + to_string(my_var[4]) + " " + to_string(new_val_2) + '\n' + to_string(my_var[6]) + to_string(my_var[7]) + " " + to_string(my_var[8]);
    AffineFile.close();

    cout << "   Affine done." << '\n';

    // Warp 
    string PATH_mask_no_pad = PATH_Output_Temp + "/mask_no_pad.nii.gz";
    command = c2d_executable + " -background 1 -create " + Size_small_target_W + "x" + Size_small_target_H + " 1x1mm -orient LP -o " + PATH_mask_no_pad;
    system(command.c_str());

    string PATH_mask_padded = PATH_Output_Temp + "/mask_padded.nii.gz";
    command = c2d_executable + " " + PATH_mask_no_pad + " -pad-to " + Size_small_target_padded_W + "x" + Size_small_target_padded_H + " 0 -o " + PATH_mask_padded;
    system(command.c_str());

    command = c2d_executable + " " + PATH_mask_padded + " -origin 0x0mm -o " + PATH_mask_padded;
    system(command.c_str());

    command = c2d_executable + " -mcs " + PATH_small_warp + " -origin 0x0mm -omc " +  PATH_small_warp;
    system(command.c_str());

    string PATH_small_warp_no_pad = PATH_Output_Temp + "/small_warp_no_pad.nii.gz";
    command = c2d_executable + " -mcs " + PATH_mask_padded + " -popas mask -mcs " + PATH_small_warp + " -foreach -push mask -times -endfor -omc " + PATH_small_warp_no_pad;
    system(command.c_str());

    string PATH_small_warp_no_pad_trim = PATH_Output_Temp + "/small_warp_no_pad_trim.nii.gz";
    command = c2d_executable + " -mcs " + PATH_small_warp_no_pad + " -foreach -trim 0vox -endfor -omc " + PATH_small_warp_no_pad_trim;
    system(command.c_str());

    string PATH_big_warp = PATH_Output + "/big_warp.nii.gz";
    command = c2d_executable + " -mcs " + PATH_small_warp_no_pad_trim + " -foreach -resample " + Size_original_target_W + "x" + Size_original_target_H + " -scale " + to_string(factor) + " -spacing 1x1mm -origin 0x0mm -endfor -omc " + PATH_big_warp;
    system(command.c_str());

    cout << "   Warp done." << '\n';
    
    if ( Flag_landmarks == 1 ){
        cout << "   Apply transformation to landmarks..." << '\n';

        ifstream data(PATH_landmarks);
        string line;
        vector<vector<string>> CSV;

        while(getline(data,line)){
            stringstream lineStream(line);
            string cell;
            vector<string> ROW;
            while (getline(lineStream,cell,',')){
                ROW.push_back(cell);
            }

            CSV.push_back(ROW);
        }

        // Convert landmarks to small resolution
        vector<vector<float>> CSV_small;
        float x, y, small_x, small_y;
        
        for ( unsigned i = 1; i < CSV.size(); i++){
            vector<float> ROW_small;
            x = stof(CSV[i][1]);
            y = stof(CSV[i][2]);

            small_x = x *1.0*stoi(Size_small_source_W)/stoi(Size_original_source_W)-0.5;
            small_y = y *1.0*stoi(Size_small_source_H)/stoi(Size_original_source_H)-0.5;
            
            ROW_small.push_back(small_x);
            ROW_small.push_back(small_y);

            CSV_small.push_back(ROW_small);
        }

        string PATH_small_landmarks = PATH_Output_Temp + "/lm_small_source.csv";
        ofstream myfile;
        myfile.precision(numeric_limits<float>::digits10);
        myfile.open(PATH_small_landmarks);

        for ( unsigned i = 0; i < CSV_small.size(); i++){
            for ( unsigned j = 0; j < CSV_small[i].size(); j++){
                myfile << CSV_small[i][j] << ",";
            }
            myfile << '\n';
        }
        myfile.close();

        // Apply transformation
        string PATH_small_warped_landmarks = PATH_Output_Temp + "/lm_small_source_warped.csv";

        string command_landmarks = greedy_executable + " -d 2 -rf " + PATH_small_source + " -rs " + PATH_small_landmarks + " " + PATH_small_warped_landmarks + " -r " + PATH_small_affine + ",-1 " + PATH_small_inv_warp;
        system(command_landmarks.c_str());

        // Convert small landmarks to full resolution
        ifstream data2(PATH_small_warped_landmarks);
        vector<vector<string>> CSV_small_warped;

        while(getline(data2,line)){
            stringstream lineStream(line);
            string cell;
            vector<string> ROW;
            while (getline(lineStream,cell,',')){
                ROW.push_back(cell);
            }

            CSV_small_warped.push_back(ROW);
        }

        vector<vector<float>> CSV_warped;
        for ( unsigned i = 1; i < CSV_small_warped.size(); i++){
            vector<float> ROW;
            small_x = stof(CSV_small_warped[i][0]);
            small_y = stof(CSV_small_warped[i][1]);

            x = (small_x+0.5)*stoi(Size_original_target_W)/stoi(Size_small_target_W);
            y = (small_y+0.5)*stoi(Size_original_target_H)/stoi(Size_small_target_H);
            
            ROW.push_back(x);
            ROW.push_back(y);

            CSV_warped.push_back(ROW);
        }

        string PATH_warped_landmarks = PATH_Output + "/warped_landmarks.csv";
        myfile.precision(numeric_limits<float>::digits10);
        myfile.open(PATH_warped_landmarks);
        myfile << ",X,Y," << '\n';

        for ( unsigned i = 0; i < CSV_warped.size(); i++){
            for ( unsigned j = 0; j < CSV_warped[i].size(); j++){
                myfile << i << "," << CSV_warped[i][j] << ",";
            }
            myfile << '\n';
        }
        cout << "   Done." << '\n';
    }
    
    if ( Flag_Full_Resolution == 1){
        // Apply full res
        // Convert source and target to niftis with good orientation, pixel spacing ,origin ..
        cout << "   Apply registration to full resolution RGB images." << '\n';
        cout << "   Adapting source..." << '\n';

        string PATH_new_source = PATH_Output_Temp + "/new_source.nii.gz";
        command = c2d_executable + " -mcs " + PATH_source + " -foreach -orient LP -spacing 1x1mm -origin 0x0mm -endfor -omc " + PATH_new_source;
        system(command.c_str());
        cout << "   Done." << '\n';

        cout << "   Adapting target..." << '\n';
        string PATH_new_target = PATH_Output_Temp + "/new_target.nii.gz";
        command = c2d_executable + " -mcs " + PATH_target + " -foreach -orient LP -spacing 1x1mm -origin 0x0mm -endfor -omc " + PATH_new_target;
        system(command.c_str());
        cout << "   Done." << '\n';

        cout << "   Applying registration..." << '\n';
        string PATH_registered_image = PATH_Output + "/registeredImage.nii.gz";
        command = greedy_executable + " -d 2 -rf " + PATH_new_target + " -rm " + PATH_new_source + " " + PATH_registered_image + " -r " + PATH_big_warp + " " + PATH_affine;
        system(command.c_str());
        cout << "   Done." << '\n';
    }

    cout << "Program finished." << '\n';

    end_script = chrono::system_clock::now();
    duration = chrono::duration_cast<chrono::seconds> (end_script-start_script).count();
    cout << "It took " << duration << " secondes to run." << '\n';
    
	return EXIT_SUCCESS;
}
