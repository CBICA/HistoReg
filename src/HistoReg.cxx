#include <iostream>
#include <fstream>

#ifdef WIN32
    #include <direct.h>
#endif

#ifdef __linux__
    #include <sys/dir.h>
#endif
#include <chrono>

#include "itkImageIOBase.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "GreedyAPI.h"

//! returns the executable path
std::string getExecutablePath()
{
#if defined(_WIN32)
  //! Initialize pointers to file and user names
  char path[FILENAME_MAX];
  GetModuleFileNameA(NULL, path, FILENAME_MAX);
  //_splitpath_s(filename, NULL, NULL, NULL, NULL, filename, NULL, NULL, NULL);
#elif __APPLE__
  char path[PATH_MAX];
  uint32_t size = PATH_MAX - 1;
  if (path != NULL)
  {
    if (_NSGetExecutablePath(path, &size) != 0)
    {
      std::cerr << "[getFullPath()] Error during getting full path..";
    }
  }
#else
  //! Initialize pointers to file and user names
  char path[PATH_MAX];
  if (::readlink("/proc/self/exe", path, sizeof(path) - 1) == -1)
    //path = dirname(path);
    std::cerr << "[getFullPath()] Error during getting full path..";
#endif

  std::string return_string = std::string(path);
  path[0] = '\0';
  return return_string;
}

int removeDirectoryRecursively(const std::string &dirname, bool bDeleteSubdirectories = true)
{
#if defined(_WIN32)
	bool bSubdirectory = false;       // Flag, indicating whether
									  // subdirectories have been found
	HANDLE hFile;                     // Handle to directory
	std::string strFilePath;          // Filepath
	std::string strPattern;           // Pattern
	WIN32_FIND_DATA FileInformation;  // File information    

	strPattern = dirname + "/*.*";
	hFile = ::FindFirstFile(strPattern.c_str(), &FileInformation);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (FileInformation.cFileName[0] != '.')
			{
				strFilePath.erase();
				strFilePath = dirname + "/" + FileInformation.cFileName;

				if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (bDeleteSubdirectories)
					{
						// Delete subdirectory
						int iRC = removeDirectoryRecursively(strFilePath, bDeleteSubdirectories);
						if (iRC)
							return iRC;
					}
					else
						bSubdirectory = true;
				}
				else
				{
					// Set file attributes
					if (::SetFileAttributes(strFilePath.c_str(),
						FILE_ATTRIBUTE_NORMAL) == FALSE)
						return ::GetLastError();

					// Delete file
					if (::DeleteFile(strFilePath.c_str()) == FALSE)
						return ::GetLastError();
				}
			}
		} while (::FindNextFile(hFile, &FileInformation) == TRUE);

		// Close handle
		::FindClose(hFile);

		DWORD dwError = ::GetLastError();
		if (dwError != ERROR_NO_MORE_FILES)
			return dwError;
		else
		{
			if (!bSubdirectory)
			{
				// Set directory attributes
				if (::SetFileAttributes(dirname.c_str(),
					FILE_ATTRIBUTE_NORMAL) == FALSE)
					return ::GetLastError();

				// Delete directory
				if (::RemoveDirectory(dirname.c_str()) == FALSE)
					return ::GetLastError();
			}
		}
	}

	return 0;
#else   
	std::string passString = "rm -rf " + dirname;
	system(passString.c_str());
#endif
	return 0;
}

using namespace std;

template <unsigned int VDim, typename TReal>
class GreedyRunner
{
public:
  static int Run(GreedyParameters &param)
  {
    // Use the threads parameter
    GreedyApproach<VDim, TReal> greedy;   
    return greedy.Run(param);
  }
};

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
#endif
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
    cerr << "Usage: " << name << " <option(s)>\n"
              << "Options:\n"
              << "\t-h,--help\t\tShow this help message\n"
              << '\n'
              << "Compulsory arguments:\n"
              << "\t-m,--moving [PATH to file]\tPATH to the moving image\n"
              << "\t-f,--fixed [PATH to file]\tPATH to the fixed image\n"
              << "\t-c,--c2d_executable [PATH to file]\tPath to c2d executable, not needed if c2d executable added to the PATH !\n"
              << "\t-o,--output [PATH to directory]\tSpecify the destination path, folder will be created if it doesn't exist.\n"
              << '\n'
              << "Optional arguments:\n"
              << "\t-l,--landmarks [PATH to file]\tPath to the source landmarks, apply registration to these landmarks.\n\t\tMust be a csv file with x,y values starting 2nd row, 2nd line.\n"
              << "\t-F,--FullResolution\tApply registration to the full resolution RGB images. Can take several minutes for large images, use '-S' to see quickly if the registration works.\n"
              << "\t-S,--SmallResolution\tApply registration to the small grayscale images\n"
              << "\t-r,--resample [VALUE]\tPercentage of the full resolution the images will be resampled to, used for computation.\n\t\tMust be a percentage but DO NOT add %. Default: 4%\n"
              << "\t-i,--iteration [VALUE]\tNumber of iteration used for initial brute search before affine registration.\n\t\tMust be a integer. Default: 5000.\n"
              << "\t-k,--kernel [VALUE]\tDefine size of the kernel, it will be the size of the resampled image divided by this value.\n\t\tMust be an integer. Small values will increase size of the kernel and so increase runtime.\n"
              << "\t-t,--tmp_directory [PATH to directory]\tPATH to temporary directory, stores temporary files, folder will be created if it doesn't exist.\n"
              << "\t-s1,--smoothing_1 (-s2,--smoothing_2) [VALUE]\tDefine the value of the smoothing parameters for deformable registration. (s1 : 'Metric gradient regularization' and s2 : 'Warp regularization')\n\t\tMust be float or integers, default value s1 = 6 and s2 = 5.\n";
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
    string s1 = "6";
    string s2 = "5";

    // executables
    string c2d_executable;
    string greedy_executable;
    
    // Flags
    int Output_provided = 0;
    int Fixed_provided = 0;
    int Moving_provided = 0;
    int c2d_executable_provided = 0;
    int Flag_Full_Resolution = 0;
    int Flag_landmarks = 0;
    int PATH_Output_Temp_provided = 0;
    int Flag_Small_Resolution = 0;
    int Flag_PNGs = 0;

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
            if ((arg == "-S") || (arg == "--SullResolution")){
                Flag_Small_Resolution = 1;
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
            if ((arg == "-s1") || (arg == "--smoothing_1")) {
				if (i + 1 < argc) { // Make sure we aren't at the end of argv!
					s1 = argv[++i];
				}
				else { // Uh-oh, there was no argument to the destination option.
					cerr << "--smoothing_1 option requires one argument." << '\n';
					return 1;
				}
			}
            if ((arg == "-s2") || (arg == "--smoothing_2")) {
				if (i + 1 < argc) { // Make sure we aren't at the end of argv!
					s2 = argv[++i];
				}
				else { // Uh-oh, there was no argument to the destination option.
					cerr << "--smoothing_2 option requires one argument." << '\n';
					return 1;
				}
			}
            if ((arg == "-P") || (arg == "--PNG")) {
				Flag_PNGs = 1;
            }
        }
    }

    // Check compulsory arguments
    if ((Output_provided == 0) || (Moving_provided == 0) || (Fixed_provided == 0)){
        cerr << "Error: Missing compulsory argument : -m, -f, -o" << '\n';
        return 1;
    }

    if ( c2d_executable_provided == 0 ){
        string PATH_exe = getExecutablePath();
        string PATH_bin = PATH_exe.substr(0,PATH_exe.length()-9);
        c2d_executable = PATH_bin + "/c3d/c2d"
        #ifdef WIN32
            +".exe"
        #endif
        ;
    }

    cout << "Done." << '\n';

    cout << "Creating output and temporary directories..." << '\n';

    // Get name images for output folder
    string name_source_full = basename (PATH_source.c_str());
    string name_target_full = basename (PATH_target.c_str());
    string delimiter = ".";

    string name_source = name_source_full.substr(0,name_source_full.find(delimiter));
    string name_target = name_target_full.substr(0,name_target_full.find(delimiter));

    // Create string for output folder
    string PATH_Output = PATH_Output_DIR + string("/") + name_source + string("_registered_to_") + name_target;
    
    if ( PATH_Output_Temp_provided == 0){
        PATH_Output_Temp = PATH_Output + "/tmp";
    }

    // NIFTIs
    string PATH_Output_niftis = PATH_Output + "/Saved_NIFTIs";
    string PATH_Output_niftis_small = PATH_Output_niftis + "/small_resolution";
    string PATH_Output_niftis_full = PATH_Output_niftis + "/full_resolution";

    // metrics
    string PATH_Output_metrics = PATH_Output + "/metrics";
    string PATH_Output_metrics_small = PATH_Output_metrics + "/small_resolution";
    string PATH_Output_metrics_full = PATH_Output_metrics + "/full_resolution";

    // PNGs
    string PATH_Output_PNGs = PATH_Output + "/PNGs";
    string PATH_Output_PNGs_small_resolution = PATH_Output_PNGs + "/small_resolution";
    string PATH_Output_PNGs_full_resolution = PATH_Output_PNGs + "/full_resolution";

    // Create output folder
	createDir(PATH_Output_DIR.c_str());
    createDir(PATH_Output.c_str());
    createDir(PATH_Output_Temp.c_str());
    createDir(PATH_Output_metrics.c_str());
	createDir(PATH_Output_metrics_small.c_str());
    createDir(PATH_Output_metrics_full.c_str());

    if (Flag_Small_Resolution){
        createDir(PATH_Output_niftis.c_str());
        createDir(PATH_Output_niftis_small.c_str());
        if (Flag_PNGs){
            createDir(PATH_Output_PNGs.c_str());
            createDir(PATH_Output_PNGs_small_resolution.c_str());
        }
    }
    if (Flag_Full_Resolution){
        createDir(PATH_Output_niftis.c_str());
        createDir(PATH_Output_niftis_full.c_str());
        if (Flag_PNGs){
            createDir(PATH_Output_PNGs.c_str());
            createDir(PATH_Output_PNGs_full_resolution.c_str());
        }
    }
    

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
    chrono::time_point<chrono::system_clock> start_intermediate, end_intermediate;
    start_intermediate = chrono::system_clock::now();

    // Compute smoothing that need to be applied before resampling : If 4% => new_size = Original_size / 25 => Kernel : 25 / 2 = 12vox
    int smoothing = 100/(2*stoi(resample));

    // Resampling c2d command.
    string Resampling_command = "-smooth-fast " + to_string(smoothing) + "x" + to_string(smoothing) + "vox -resample " + resample + "% -spacing 1x1mm -orient LP -origin 0x0mm -o";

    string PATH_small_target = PATH_Output_Temp + string("/new_small_target.nii.gz");
    string PATH_small_source = PATH_Output_Temp + string("/new_small_source.nii.gz");

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
    int kernel_W = stoi(Size_small_target_W) / stoi(Kernel_Divider);
    int kernel_H = stoi(Size_small_target_H) / stoi(Kernel_Divider);

    // Makes the kernel a square
    int kernel = kernel_H; 
    if ( kernel_H < kernel_W )
    {
        kernel = kernel_W;
    }

    int Size_W_minus_kernel_target = stoi(Size_small_target_W) - kernel;
    int Size_H_minus_kernel_target = stoi(Size_small_target_H) - kernel;

	string grep;
#ifdef WIN32
	grep = " | findstr /C:\" 1 \"";
#else
	grep = " | grep \" 1 \"";
#endif

    // We want to pad with intensity as close as the background as possible.
    // Extract mean and std of the intensities of the background (4 square of the size of the kernel at each corners)
    
    // Create the command to get the intensites of the corners
	string Four_corners_command = c2d_executable + string(" ") + PATH_small_target + string(" -dup -cmv -popas Y -popas X -push X -thresh 0 ") + to_string(kernel) + string(" 1 0 -push Y -thresh 0 ") + to_string(kernel) + string(" 1 0 -times -popas c00 -push X -thresh ") + to_string(Size_W_minus_kernel_target) + string(" ") + Size_small_target_W + string(" 1 0 -push Y -thresh 0 ") + to_string(kernel) + string(" 1 0 -times -popas c01 -push X -thresh ") + to_string(Size_W_minus_kernel_target) + string(" ") + Size_small_target_W + string(" 1 0 -push Y -thresh ") + to_string(Size_H_minus_kernel_target) + string(" ") + Size_small_target_H + string(" 1 0 -times -popas c11 -push X -thresh 0 ") + to_string(kernel) + string(" 1 0 -push Y -thresh ") + to_string(Size_H_minus_kernel_target) + string(" ") + Size_small_target_H + string(" 1 0 -times -popas c10 -push c00 -push c01 -push c11 -push c10 -add -add -add -lstat" + grep);

    // run command and get output
    string stats_target = GetStdoutFromCommand(Four_corners_command);

    // Replace multiple consecutive space in the string by only one (this is needed because of how the ouput of the c2d command is)
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

    // command to get intensities
    Four_corners_command = c2d_executable + string(" ") + PATH_small_source + string(" -dup -cmv -popas Y -popas X -push X -thresh 0 ") + to_string(kernel) + string(" 1 0 -push Y -thresh 0 ") + to_string(kernel) + string(" 1 0 -times -popas c00 -push X -thresh ") + to_string(Size_W_minus_kernel_source) + string(" ") + Size_small_source_W + string(" 1 0 -push Y -thresh 0 ") + to_string(kernel) + string(" 1 0 -times -popas c01 -push X -thresh ") + to_string(Size_W_minus_kernel_source) + string(" ") + Size_small_source_W + string(" 1 0 -push Y -thresh ") + to_string(Size_H_minus_kernel_source) + string(" ") + Size_small_source_H + string(" 1 0 -times -popas c11 -push X -thresh 0 ") + to_string(kernel) + string(" 1 0 -push Y -thresh ") + to_string(Size_H_minus_kernel_source) + string(" ") + Size_small_source_H + string(" 1 0 -times -popas c10 -push c00 -push c01 -push c11 -push c10 -add -add -add -lstat" + grep);
	
    // run command and get output
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

    // Computes 4 times the size of the kernel for futur padding. Pad with 4 times the size of the kernel to be sure that the ROI (tissues) is far enough from boundaries.
    int four_kernel = 4*kernel;

    string PATH_small_target_padded;
    string PATH_small_source_padded;

    // If flag to apply transformation on small images, save these in the output directory, otherwise save them in the temporary directory
    if ( Flag_Small_Resolution == 1 ){
        PATH_small_target_padded = PATH_Output_niftis_small + string("/new_small_target_padded.nii.gz");
        PATH_small_source_padded = PATH_Output_niftis_small + string("/new_small_source_padded.nii.gz");
    }
    else{
        PATH_small_target_padded = PATH_Output_Temp + string("/new_small_target_padded.nii.gz");
        PATH_small_source_padded = PATH_Output_Temp + string("/new_small_source_padded.nii.gz");
    }

    string New_size_H = Size_small_target_H;
    string New_size_W = Size_small_target_W;

    // Compare source and target size and pad images to match their size
    if ( Size_small_source_H != Size_small_target_H || Size_small_source_W != Size_small_target_W )
    {
        // Compare size of both images and keep the larger
        //cout << "   Modifying sizes..." << '\n';
        if ( Size_small_target_W < Size_small_source_W )
        {
            New_size_W = Size_small_source_W;
        }
        if ( Size_small_target_H < Size_small_source_H )
        {
            New_size_H = Size_small_source_H;
        }

	    // First padding with zeros.
	    // pad images to the desired size with zeros, if one is already of the desired size the command will not modify it
        //cout << "   New size is : " << New_size_W << "x" << New_size_H << '\n';

        string command_resize_target = c2d_executable + string(" ") + PATH_small_target + string(" -pad-to ") + New_size_W + string("x") + New_size_H + string(" 0 -o ") + PATH_small_target_padded;
        string command_resize_source = c2d_executable + string(" ") + PATH_small_source + string(" -pad-to ") + New_size_W + string("x") + New_size_H + string(" 0 -o ") + PATH_small_source_padded;
      
        system(command_resize_target.c_str());
        system(command_resize_source.c_str());
    }
    else
    {
        PATH_small_target_padded = PATH_small_target;
        PATH_small_source_padded = PATH_small_source;
    }

    // We don't want to pad with zeros but with intensity close to the background.
    // Target
    // a/
    // Create a mask separating original images from its padded part (0 for padded pixels and 1 for original ones)
    // Then pad it with 0 by 4 times the kernel to be sure that ROI far enough from the boundaries of the images.
    string PATH_mask_target = PATH_Output_Temp + string("/mask_target.nii.gz");
    string command = c2d_executable + string(" ") + PATH_small_target_padded + string(" -thresh 1 inf 1 0 -pad ") + to_string(four_kernel) + string("x") + to_string(four_kernel) + string(" ") + to_string(four_kernel) + string("x") + to_string(four_kernel) + string(" 0 -o ") + PATH_mask_target;
    system(command.c_str());

    // b/
    // Inverse this mask (1 for padded pixels and 0 for original pixels)
    // Replace every pixels values in the first mask by 1
    // Then scale this mask by the mean of the intenistites in the 4 corners and add a gaussian noise of the standart deviation of this intensity.
    // Finaly multiply both mask together so final result have intensity 0 for each pixels that belongs to the original images and intensity close to the background for each padded pixels.
    command = c2d_executable + string(" ") + PATH_mask_target + string(" -replace 0 1 1 0 -popas invmask ") + PATH_mask_target + string(" -replace 0 1 1 1 -scale ") + mean_target + string(" -noise-gaussian ") + std_target + string(" -push invmask -times -o ") + PATH_mask_target;
    system(command.c_str());

    // c/
    // Pad the target image with 0 by 4 times the size of the kernels so it has the same size as the mask we just computed.
    command = c2d_executable + string(" ") + PATH_small_target_padded + string(" -pad ") + to_string(four_kernel) + string("x") + to_string(four_kernel) + string(" ") + to_string(four_kernel) + string("x") + to_string(four_kernel) + string(" 0 -o ") + PATH_small_target_padded;
    system(command.c_str());

    // d/
    // Add mask to the image padded with zeros, so it is now padded with the mean of the intensities in the four corners + a gaussian noise of the standart deviation of this intensity.
    command = c2d_executable + string(" ") + PATH_mask_target + string(" ") + PATH_small_target_padded + string(" -add -o ") + PATH_small_target_padded;
    system(command.c_str());

    cout << "   Target done." << '\n';


    // Same idea with the source
    // a/
    string PATH_mask_source = PATH_Output_Temp + string("/mask_source.nii.gz");
    command = c2d_executable + string(" ") + PATH_small_source_padded + string(" -thresh 1 inf 1 0 -pad ") + to_string(four_kernel) + string("x") + to_string(four_kernel) + string(" ") + to_string(four_kernel) + string("x") + to_string(four_kernel) + string(" 0 -o ") + PATH_mask_source;
    system(command.c_str());

    // b/
    command = c2d_executable + " " + PATH_mask_source + string(" -replace 0 1 1 0 -popas invmask ") + PATH_mask_source + string(" -replace 0 1 1 1 -scale ") + mean_source + string(" -noise-gaussian ") + std_source + string(" -push invmask -times -o ") + PATH_mask_source;
    system(command.c_str());

    // c/
    command = c2d_executable + " " + PATH_small_source_padded + string(" -pad ") + to_string(four_kernel) + string("x") + to_string(four_kernel) + string(" ") + to_string(four_kernel) + string("x") + to_string(four_kernel) + string(" 0 -o ") + PATH_small_source_padded;
    system(command.c_str());

    // d/
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

    end_intermediate = chrono::system_clock::now();
    duration = chrono::duration_cast<chrono::seconds> (end_intermediate-start_intermediate).count();
    cout << "Preprocessing took : " << duration << " secondes." << '\n';

    // Print sizes
    // cout << "   Small_target_W : " << Size_small_target_padded_W << '\n';
    // cout << "   Small_target_H : " << Size_small_target_padded_H << '\n';

    cout << "Registration..." << '\n';
    cout << "   Computing affine..." << '\n';

    int offset = (stoi(Size_small_target_H) + four_kernel) / 10;
    
    // Registration
    // Affine
    start_intermediate = chrono::system_clock::now();

    // Define kernel
    vector<int> kernel_radius;
    kernel_radius.push_back(kernel);
    kernel_radius.push_back(kernel);

    // Define number of iteration (100x50x10)
    vector<int> iteration_vector;
    iteration_vector.push_back(100);
    iteration_vector.push_back(50);
    iteration_vector.push_back(10);

    // Define greedy parameters
    GreedyParameters param_Aff;
    GreedyParameters::SetToDefaults(param_Aff);

    // Define mode and dimension
    param_Aff.mode = GreedyParameters::AFFINE;
    param_Aff.affine_dof = GreedyParameters::DOF_AFFINE;
    param_Aff.dim = 2;

    // Define metric and kernel
    param_Aff.metric = GreedyParameters::NCC;
    param_Aff.metric_radius = kernel_radius;

    // Define number of iteration
    param_Aff.iter_per_level = iteration_vector;
    
    // Define source and target
    ImagePairSpec ip;
    ip.fixed = PATH_small_target_padded;
    ip.moving = PATH_small_source_padded;
    param_Aff.inputs.push_back(ip);

    // Define parameters for brute search
    param_Aff.rigid_search.mode = RANDOM_NORMAL_ROTATION;
    param_Aff.rigid_search.sigma_angle = 180;
    param_Aff.rigid_search.iterations = stoi(iteration);
    param_Aff.rigid_search.sigma_xyz = offset;

    // Define kernel for mask
    param_Aff.gradient_mask_trim_radius = kernel_radius;

    // Define output
     string PATH_small_affine = PATH_Output_metrics_small + string("/small_Affine.mat");
    param_Aff.output = PATH_small_affine;

    // Run affine
    GreedyRunner<2, double>::Run(param_Aff);



    // Diffeomorphic
    cout << "   Computing Diffeomorphic..." << '\n';

    // Define affine transformation
    TransformSpec TransformAff;
    TransformAff.filename = PATH_small_affine;

    // Reset param
    GreedyParameters param_Diff;
    GreedyParameters::SetToDefaults(param_Diff);

    // Define dimension and mode
    param_Diff.mode = GreedyParameters::GREEDY;
    param_Diff.dim = 2;

    // Define metric and kernel
    param_Diff.metric = GreedyParameters::NCC;
    param_Diff.metric_radius = kernel_radius;

    // Define source and target
    param_Diff.inputs.push_back(ip);

    // Define affine transformation to be applied before diffeomorphic registration
    param_Diff.moving_pre_transforms.push_back(TransformAff);

    // Defin number of iteration (100x50x10)
    param_Diff.iter_per_level = iteration_vector;

    // Define smoothing parameters
    //SmoothingParameters sigma_pre, sigma_post;
    param_Diff.sigma_pre.sigma = stod(s1);
    param_Diff.sigma_post.sigma = stod(s2);

    // Define output
    string PATH_small_warp = PATH_Output_metrics_small + string("/small_warp.nii.gz");
    string PATH_small_inv_warp = PATH_Output_metrics_small + string("/small_inv_warp.nii.gz");

    param_Diff.inverse_warp = PATH_small_inv_warp;
    param_Diff.output = PATH_small_warp;

    // Run Diffeomorphic registration
    GreedyRunner<2, double>::Run(param_Diff);

    end_intermediate = chrono::system_clock::now();
    duration = chrono::duration_cast<chrono::seconds> (end_intermediate-start_intermediate).count();
    cout << "Registration took : " << duration << " secondes." << '\n';

    // Define interpolation as linear, same idea as transformation
    ResliceSpec Reslices_images;
    InterpSpec Interp;
    Interp.mode = InterpSpec::InterpMode::LINEAR;

    if ( Flag_Small_Resolution == 1){
        start_intermediate = chrono::system_clock::now();
        // Apply to small images
        cout << "   Applying to small grayscale images..." << '\n';
        // Reset param
        GreedyParameters param_reslice;
        GreedyParameters::SetToDefaults(param_reslice);

        // Define dimension and mode
        param_reslice.dim = 2;
        param_reslice.mode = GreedyParameters::RESLICE;


        // Define diffeomorphic transformation
        vector<TransformSpec> Transformations;
        TransformSpec TransformDiff;
        TransformDiff.filename = PATH_small_warp;

        // Define transformations
        Transformations.push_back(TransformDiff);
        Transformations.push_back(TransformAff);

        param_reslice.reslice_param.transforms = Transformations;

        // Define source, target, interpolation, and output
        Reslices_images.moving = PATH_small_source_padded;
        Reslices_images.interp = Interp;
        string PATH_small_registered_image = PATH_Output_niftis_small + string("/small_registeredImage.nii.gz");
        Reslices_images.output = PATH_small_registered_image;
        param_reslice.reslice_param.images.push_back(Reslices_images);
        param_reslice.reslice_param.ref_image = PATH_small_target_padded;

        // Run reslice
        GreedyRunner<2, double>::Run(param_reslice);

        end_intermediate = chrono::system_clock::now();
        duration = chrono::duration_cast<chrono::seconds> (end_intermediate-start_intermediate).count();
        cout << "Apply transformation on small image took : " << duration << " secondes." << '\n';

        // Converting to PNGs
        if ( Flag_PNGs == 1 ){
            start_intermediate = chrono::system_clock::now();

            cout << "Converting images into PNGs..." << '\n';
            string PATH_small_target_png = PATH_Output_PNGs_small_resolution + "/new_target.png";
            command = c2d_executable  + " -mcs " + PATH_small_target_padded + " -foreach -type uchar -endfor -omc " + PATH_small_target_png;
            system(command.c_str());
            cout << "   Target done..." << '\n';

            string PATH_small_source_png = PATH_Output_PNGs_small_resolution + "/new_source.png";
            command = c2d_executable + " -mcs " + PATH_small_source_padded + " -foreach -type uchar -endfor -omc " + PATH_small_source_png;
            system(command.c_str());
            cout << "   Source done..." << '\n';

            string PATH_small_registered_image_png = PATH_Output_PNGs_small_resolution + "/registeredImage.png";
            command = c2d_executable + " -mcs " + PATH_small_registered_image + " -foreach -type uchar -endfor -omc " + PATH_small_registered_image_png;
            system(command.c_str());
            cout << "   Registered image done." << '\n';

            end_intermediate = chrono::system_clock::now();
            duration = chrono::duration_cast<chrono::seconds> (end_intermediate-start_intermediate).count();
            cout << "Converting result to PNGs images took : " << duration << " secondes." << '\n';
        }
    }

    cout << "Adaptating registration mectrics to non-padded full resolution RGB images..." << '\n';
    start_intermediate = chrono::system_clock::now();
    // Compute metrics for full resolution images
    // Affine
    // Adapt affine matrix to full resolution, multiply translation vector of the matrix by the scale we resampled the image. For example with 4%, size of the image is divided by 25 so we multiply the translation part of the matrix by 25. 
    // Rotation scaling and shearing stay the same.
    ifstream smallAffFile;
    string STRING;
    int factor = 100/stoi(resample);
    long double my_var[9];
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
    string PATH_affine = PATH_Output_metrics_full + "/Affine.mat";
    ofstream AffineFile;
    AffineFile.open(PATH_affine);
    AffineFile << to_string(my_var[0]) + " " + to_string(my_var[1]) + " " + to_string(new_val_1) + '\n' + to_string(my_var[3]) + " " + to_string(my_var[4]) + " " + to_string(new_val_2) + '\n' + to_string(my_var[6]) + to_string(my_var[7]) + " " + to_string(my_var[8]);
    AffineFile.close();

    cout << "   Affine done." << '\n';

    // Warp 
    // We want to apply transformation to the original images that are NOT PADDED.
    
    // Create a 1 intensity image of the size of the full resolution target without padding
    string PATH_mask_no_pad = PATH_Output_Temp + "/mask_no_pad.nii.gz";
    command = c2d_executable + " -background 1 -create " + Size_small_target_W + "x" + Size_small_target_H + " 1x1mm -orient LP -o " + PATH_mask_no_pad;
    system(command.c_str());


    // Pad this image with 0 to the size of the full resolution target after padding. We have now a mask with intensity 1 for pixel that belongs to the original target image and intensity 0 for pixels that belogns to the padded part of the image.
    string PATH_mask_padded = PATH_Output_Temp + "/mask_padded.nii.gz";
    command = c2d_executable + " " + PATH_mask_no_pad + " -pad-to " + Size_small_target_padded_W + "x" + Size_small_target_padded_H + " 0 -o " + PATH_mask_padded;
    system(command.c_str());

    // Change origin of the mask so it matches the one of the full resolution warp image. 
    command = c2d_executable + " " + PATH_mask_padded + " -origin 0x0mm -o " + PATH_mask_padded;
    system(command.c_str());

    // Change origin of the mask so it matches the one of the full resolution warp image. 
    command = c2d_executable + " -mcs " + PATH_small_warp + " -origin 0x0mm -omc " +  PATH_small_warp;
    system(command.c_str());

    // Multiply the mask and the full resolution warp image to force to 0 every value in the padded part of the warp image.
    string PATH_small_warp_no_pad = PATH_Output_Temp + "/small_warp_no_pad.nii.gz";
    command = c2d_executable + " -mcs " + PATH_mask_padded + " -popas mask -mcs " + PATH_small_warp + " -foreach -push mask -times -endfor -omc " + PATH_small_warp_no_pad;
    system(command.c_str());

    // trim the warp image to remove every pixels with intensity 0 that are on the border of the image, ie every pixels that belongs to the padded part of the warp image.
    string PATH_small_warp_no_pad_trim = PATH_Output_Temp + "/small_warp_no_pad_trim.nii.gz";
    command = c2d_executable + " -mcs " + PATH_small_warp_no_pad + " -foreach -trim 0vox -endfor -omc " + PATH_small_warp_no_pad_trim;
    system(command.c_str());

    // resample warp to original image dimension and scale it with the scale we resampled the image to. (the warp image is a matrix that contains a translation vector for each pixel of the target image, we need to scale this translation to the new resolution as we did for the affine matrix)
    string PATH_big_warp = PATH_Output_metrics_full + "/big_warp.nii.gz";
    command = c2d_executable + " -mcs " + PATH_small_warp_no_pad_trim + " -foreach -resample " + Size_original_target_W + "x" + Size_original_target_H + " -scale " + to_string(factor) + " -spacing 1x1mm -origin 0x0mm -endfor -omc " + PATH_big_warp;
    system(command.c_str());

    cout << "   Warp done." << '\n';
    end_intermediate = chrono::system_clock::now();
    duration = chrono::duration_cast<chrono::seconds> (end_intermediate-start_intermediate).count();
    cout << "Adapt transformation to original image took : " << duration << " secondes." << '\n';
    
    if ( Flag_landmarks == 1 ){
        cout << "   Apply transformation to landmarks..." << '\n';
        start_intermediate = chrono::system_clock::now();

        // read landmarks
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

            small_x = x*1.0*stoi(Size_small_source_W)/stoi(Size_original_source_W)-0.5;
            small_y = y*1.0*stoi(Size_small_source_H)/stoi(Size_original_source_H)-0.5;
            
            ROW_small.push_back(small_x);
            ROW_small.push_back(small_y);

            CSV_small.push_back(ROW_small);
        }

        // Save converted landmarks to a csv file
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

        // Apply transformation to landmarks
        // Output
        string PATH_small_warped_landmarks = PATH_Output_Temp + "/lm_small_source_warped.csv";

        // reset param
        GreedyParameters param_lm;
        GreedyParameters::SetToDefaults(param_lm);

        // Define dim en mode
        param_lm.dim = 2;
        param_lm.mode = GreedyParameters::RESLICE;

        // Define source, target and output
        ResliceMeshSpec Reslices_lm_full;
        Reslices_lm_full.fixed = PATH_small_landmarks;
        Reslices_lm_full.output = PATH_small_warped_landmarks;
        param_lm.reslice_param.meshes.push_back(Reslices_lm_full);
        param_lm.reslice_param.ref_image = PATH_small_source;

        // Define transofrmations
        vector<TransformSpec> Transformations_lm;
        TransformSpec TransformDiffLM, TransformAffLM;
        TransformDiffLM.filename = PATH_small_inv_warp;
        TransformAffLM.filename = PATH_small_affine;
        TransformAffLM.exponent = -1;

        Transformations_lm.push_back(TransformAffLM);
        Transformations_lm.push_back(TransformDiffLM);
        param_lm.reslice_param.transforms = Transformations_lm;

        // Run
        GreedyRunner<2, double>::Run(param_lm);

        // Read landmarks after transformation
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

        // Convert small landmarks to full resolution
        vector<vector<float>> CSV_warped;
        for ( unsigned i = 0; i < CSV_small_warped.size(); i++){
            vector<float> ROW;
            small_x = stof(CSV_small_warped[i][0]);
            small_y = stof(CSV_small_warped[i][1]);

            x = (small_x+0.5)*stoi(Size_original_target_W)/stoi(Size_small_target_W);
            y = (small_y+0.5)*stoi(Size_original_target_H)/stoi(Size_small_target_H);
            
            ROW.push_back(x);
            ROW.push_back(y);

            CSV_warped.push_back(ROW);
        }

        // Save converted landmarks to a csv file
        string PATH_warped_landmarks = PATH_Output + "/warped_landmarks.csv";
        ofstream myfile_2;
        myfile_2.precision(numeric_limits<float>::digits10);
        myfile_2.open(PATH_warped_landmarks);
        myfile_2 << ",X,Y," << '\n';

        for ( unsigned i = 0; i < CSV_warped.size(); i++){
            myfile_2 << i << ",";
            for ( unsigned j = 0; j < CSV_warped[i].size(); j++){
                myfile_2 << CSV_warped[i][j] << ",";
                }
            myfile_2 << '\n';
        }
        cout << "   Done." << '\n';
        end_intermediate = chrono::system_clock::now();
        duration = chrono::duration_cast<chrono::seconds> (end_intermediate-start_intermediate).count();
        cout << "Apply transformation to landmarks took : " << duration << " secondes." << '\n';
    }
    
    if ( Flag_Full_Resolution == 1){
        start_intermediate = chrono::system_clock::now();
        // Apply full res
        // Convert source and target to niftis with good orientation, pixel spacing ,origin ..
        cout << "   Apply registration to full resolution RGB images." << '\n';
        cout << "   Adapting source..." << '\n';

        string PATH_new_source = PATH_Output_niftis_full + "/new_source.nii.gz";
        command = c2d_executable + " -mcs " + PATH_source + " -foreach -orient LP -spacing 1x1mm -origin 0x0mm -endfor -omc " + PATH_new_source;
        system(command.c_str());
        cout << "   Done." << '\n';

        cout << "   Adapting target..." << '\n';
        string PATH_new_target = PATH_Output_niftis_full + "/new_target.nii.gz";
        command = c2d_executable + " -mcs " + PATH_target + " -foreach -orient LP -spacing 1x1mm -origin 0x0mm -endfor -omc " + PATH_new_target;
        system(command.c_str());
        cout << "   Done." << '\n';

        cout << "   Applying registration..." << '\n';
        string PATH_registered_image = PATH_Output_niftis_full + "/registeredImage.nii.gz";

        // Reset param
        GreedyParameters param_reslice_full;
        GreedyParameters::SetToDefaults(param_reslice_full);

        // Define dimension and mode
        param_reslice_full.dim = 2;
        param_reslice_full.mode = GreedyParameters::RESLICE;

        // Define transformation to be applied
        ResliceSpec Reslices_images_full;
        vector<TransformSpec> Transformations_full;
        TransformSpec TransformDiffFull, TransformAffFull;

        TransformDiffFull.filename = PATH_big_warp;
        TransformAffFull.filename = PATH_affine;

        Transformations_full.push_back(TransformDiffFull);
        Transformations_full.push_back(TransformAffFull);
        param_reslice_full.reslice_param.transforms = Transformations_full;

        // Define source, target, interpolation mode and output
        Reslices_images_full.moving = PATH_new_source;
        Reslices_images_full.interp = Interp;
        Reslices_images_full.output = PATH_registered_image;

        param_reslice_full.reslice_param.images.push_back(Reslices_images_full);
        param_reslice_full.reslice_param.ref_image = PATH_new_target;

        // Run greedy
        GreedyRunner<2, double>::Run(param_reslice_full);
        cout << "   Done." << '\n';
        end_intermediate = chrono::system_clock::now();
        duration = chrono::duration_cast<chrono::seconds> (end_intermediate-start_intermediate).count();
        cout << "Apply transformation to original images took : " << duration << " secondes." << '\n';
    
        // Converting to PNGs
        if ( Flag_PNGs == 1 ){
            start_intermediate = chrono::system_clock::now();

            cout << "Converting images into PNGs..." << '\n';
            string PATH_new_target_png = PATH_Output_PNGs_full_resolution + "/new_target.png";
            command = c2d_executable + " -mcs " + PATH_new_target + " -foreach -type uchar -endfor -omc " + PATH_new_target_png;
            system(command.c_str());
            cout << "   Target done..." << '\n';

            string PATH_new_source_png = PATH_Output_PNGs_full_resolution + "/new_source.png";
            command = c2d_executable + " -mcs " + PATH_new_source + " -foreach -type uchar -endfor -omc " + PATH_new_source_png;
            system(command.c_str());
            cout << "   Source done..." << '\n';

            string PATH_registered_image_png = PATH_Output_PNGs_full_resolution + "/registeredImage.png";
            command = c2d_executable + " -mcs " + PATH_registered_image + " -foreach -type uchar -endfor -omc " + PATH_registered_image_png;
            system(command.c_str());
            cout << "   Registered image done." << '\n';

            end_intermediate = chrono::system_clock::now();
            duration = chrono::duration_cast<chrono::seconds> (end_intermediate-start_intermediate).count();
            cout << "Converting result to PNGs images took : " << duration << " secondes." << '\n';
        }
    }

    cout << "Removing temporary directory..." << '\n';

	removeDirectoryRecursively(PATH_Output_Temp);

    cout << "Program finished." << '\n';

    end_script = chrono::system_clock::now();
    duration = chrono::duration_cast<chrono::seconds> (end_script-start_script).count();
    cout << "It took " << duration << " secondes to run." << '\n';
    
	return EXIT_SUCCESS;
}
