// Copyright (c) 2014-2020 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#pragma once

#include "GitSourceControlRevision.h"
#include "GitSourceControlState.h"
#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MAJOR_VERSION == 5
#include "UObject/ObjectSaveContext.h"
#endif

class FGitSourceControlState;

class FGitSourceControlCommand;

/**
 * Helper struct for maintaining temporary files for passing to commands
 */
class FGitScopedTempFile
{
public:

	/** Constructor - open & write string to temp file */
	FGitScopedTempFile(const FText& InText);

	/** Destructor - delete temp file */
	~FGitScopedTempFile();

	/** Get the filename of this temp file - empty if it failed to be created */
	const FString& GetFilename() const;

private:
	/** The filename we are writing to */
	FString Filename;
};

struct FGitVersion;

class FGitLockedFilesCache
{
public:
	static FDateTime LastUpdated;

 static const TMap<FString, FString>& GetLockedFiles() { return LockedFiles; }
 static void SetLockedFiles(const TMap<FString, FString>& newLocks);
 static void AddLockedFile(const FString& filePath, const FString& lockUser);
 static void RemoveLockedFile(const FString& filePath);

private:
 static void OnFileLockChanged(const FString& filePath, const FString& lockUser, bool locked);
 // update local read/write state when our own lock statuses change
	static TMap<FString, FString> LockedFiles;
};

namespace GitSourceControlUtils
{
	/**
		*  Returns an updated repo root if all selected files are in a plugin subfolder, and the plugin subfolder is a git repo
		*  This supports the case where each plugin is a sub module
		*
		* @param AbsoluteFilePaths		The list of files in the SC operation
		* @param PathToRepositoryRoot	The original path to the repository root (used by default)
		*/
	FString ChangeRepositoryRootIfSubmodule(TArray<FString>& AbsoluteFilePaths, const FString& PathToRepositoryRoot);

	/**
		*  Returns an updated repo root if all selected file is in a plugin subfolder, and the plugin subfolder is a git repo
		*  This supports the case where each plugin is a sub module
		*
		* @param AbsoluteFilePath		The file in the SC operation
		* @param PathToRepositoryRoot	The original path to the repository root (used by default)
		*/
	FString ChangeRepositoryRootIfSubmodule(FString & AbsoluteFilePath, const FString& PathToRepositoryRoot);

/**
 * Find the path to the Git binary, looking into a few places (standalone Git install, and other common tools embedding Git)
 * @returns the path to the Git binary if found, or an empty string.
 */
FString FindGitBinaryPath();

/**
 * Run a Git "version" command to check the availability of the binary.
 * @param InPathToGitBinary		The path to the Git binary
 * @param OutGitVersion         If provided, populate with the git version parsed from "version" command
 * @returns true if the command succeeded and returned no errors
 */
bool CheckGitAvailability(const FString& InPathToGitBinary, FGitVersion* OutVersion = nullptr);

/**
 * Parse the output from the "version" command into GitMajorVersion and GitMinorVersion.
 * @param InVersionString       The version string returned by `git --version`
 * @param OutVersion            The FGitVersion to populate
 */
 void ParseGitVersion(const FString& InVersionString, FGitVersion* OutVersion);

	/**
		* Check git for various optional capabilities by various means.
		* @param InPathToGitBinary		The path to the Git binary
		* @param OutGitVersion			If provided, populate with the git version parsed from "version" command
		*/
	void FindGitCapabilities(const FString& InPathToGitBinary, FGitVersion* OutVersion);

	/**
		* Run a Git "lfs" command to check the availability of the "Large File System" extension.
		* @param InPathToGitBinary		The path to the Git binary
		* @param OutGitVersion			If provided, populate with the git version parsed from "version" command
		*/
	void FindGitLfsCapabilities(const FString& InPathToGitBinary, FGitVersion* OutVersion);

/**
 * Find the root of the Git repository, looking from the provided path and upward in its parent directories
 * @param InPath				The path to the Game Directory (or any path or file in any git repository)
 * @param OutRepositoryRoot		The path to the root directory of the Git repository if found, else the path to the ProjectDir
 * @returns true if the command succeeded and returned no errors
 */
bool FindRootDirectory(const FString& InPath, FString& OutRepositoryRoot);

/**
 * Get Git config user.name & user.email
 * @param	InPathToGitBinary	The path to the Git binary
 * @param	InRepositoryRoot	The Git repository from where to run the command - usually the Game directory (can be empty)
 * @param	OutUserName			Name of the Git user configured for this repository (or globaly)
 * @param	OutEmailName		E-mail of the Git user configured for this repository (or globaly)
 */
void GetUserConfig(const FString& InPathToGitBinary, const FString& InRepositoryRoot, FString& OutUserName, FString& OutUserEmail);

/**
 * Get Git current checked-out branch
 * @param	InPathToGitBinary	The path to the Git binary
 * @param	InRepositoryRoot	The Git repository from where to run the command - usually the Game directory
 * @param	OutBranchName		Name of the current checked-out branch (if any, ie. not in detached HEAD)
 * @returns true if the command succeeded and returned no errors
 */
bool GetBranchName(const FString& InPathToGitBinary, const FString& InRepositoryRoot, FString& OutBranchName);

/**
 * Get Git remote tracking branch
 * @returns false if the branch is not tracking a remote
 */
bool GetRemoteBranchName(const FString& InPathToGitBinary, const FString& InRepositoryRoot, FString& OutBranchName);

 /**
 * Get Git remote tracking branches that match wildcard
 * @returns false if no matching branches
 */
 bool GetRemoteBranchesWildcard(const FString& InPathToGitBinary, const FString& InRepositoryRoot, const FString& PatternMatch, TArray<FString>& OutBranchNames);
 
/**
 * Get Git current commit details
 * @param	InPathToGitBinary	The path to the Git binary
 * @param	InRepositoryRoot	The Git repository from where to run the command - usually the Game directory
 * @param	OutCommitId			Current Commit full SHA1
 * @param	OutCommitSummary	Current Commit description's Summary
 * @returns true if the command succeeded and returned no errors
 */
bool GetCommitInfo(const FString& InPathToGitBinary, const FString& InRepositoryRoot, FString& OutCommitId, FString& OutCommitSummary);

/**
 * Get the URL of the "origin" defaut remote server
 * @param	InPathToGitBinary	The path to the Git binary
 * @param	InRepositoryRoot	The Git repository from where to run the command - usually the Game directory
 * @param	OutRemoteUrl		URL of "origin" defaut remote server
 * @returns true if the command succeeded and returned no errors
 */
bool GetRemoteUrl(const FString& InPathToGitBinary, const FString& InRepositoryRoot, FString& OutRemoteUrl);

/**
 * Run a Git command - output is a string TArray.
 *
 * @param	InCommand			The Git command - e.g. commit
 * @param	InPathToGitBinary	The path to the Git binary
 * @param	InRepositoryRoot	The Git repository from where to run the command - usually the Game directory (can be empty)
 * @param	InParameters		The parameters to the Git command
 * @param	InFiles				The files to be operated on
 * @param	OutResults			The results (from StdOut) as an array per-line
 * @param	OutErrorMessages	Any errors (from StdErr) as an array per-line
 * @returns true if the command succeeded and returned no errors
 */
GITSOURCECONTROL_API  bool RunCommand( const FString & InCommand, const FString & InPathToGitBinary, const FString & InRepositoryRoot, const TArray< FString > & InParameters, const TArray< FString > & InFiles, TArray< FString > & OutResults, TArray< FString > & OutErrorMessages );
bool RunCommandInternalRaw(const FString& InCommand, const FString& InPathToGitBinary, const FString& InRepositoryRoot, const TArray<FString>& InParameters, const TArray<FString>& InFiles, FString& OutResults, FString& OutErrors, const int32 ExpectedReturnCode = 0);

/**
 * Unloads packages of specified named files
 */
TArray<class UPackage*> UnlinkPackages(const TArray<FString>& InPackageNames);

/**
 * Reloads packages for these packages
 */
void ReloadPackages(TArray<UPackage*>& InPackagesToReload);

/**
 * Gets all Git tracked files, including within directories, recursively
 */
bool ListFilesInDirectoryRecurse(const FString& InPathToGitBinary, const FString& InRepositoryRoot, const FString& InDirectory, TArray<FString>& OutFiles);

/**
 * Run a Git "commit" command by batches.
 *
 * @param	InPathToGitBinary	The path to the Git binary
 * @param	InRepositoryRoot	The Git repository from where to run the command - usually the Game directory
 * @param	InParameter			The parameters to the Git commit command
 * @param	InFiles				The files to be operated on
 * @param	OutErrorMessages	Any errors (from StdErr) as an array per-line
 * @returns true if the command succeeded and returned no errors
 */
bool RunCommit(const FString& InPathToGitBinary, const FString& InRepositoryRoot, const TArray<FString>& InParameters, const TArray<FString>& InFiles, TArray<FString>& OutResults, TArray<FString>& OutErrorMessages);

/**
 * @brief Detects how to parse the result of a "status" command to get workspace file states
 *
 *  It is either a command for a whole directory (ie. "Content/", in case of "Submit to Revision Control" menu),
 * or for one or more files all on a same directory (by design, since we group files by directory in RunUpdateStatus())
 *
 * @param[in]	InPathToGitBinary	The path to the Git binary
 * @param[in]	InRepositoryRoot	The Git repository from where to run the command - usually the Game directory (can be empty)
 * @param[in]	InUsingLfsLocking	Tells if using the Git LFS file Locking workflow
 * @param[in]	InFiles				List of files in a directory, or the path to the directory itself (never empty).
 * @param[out]	InResults			Results from the "status" command
 * @param[out]	OutStates			States of files for witch the status has been gathered (distinct than InFiles in case of a "directory status")
 */
GITSOURCECONTROL_API void ParseStatusResults( const FString & InPathToGitBinary, const FString & InRepositoryRoot, const bool InUsingLfsLocking, const TArray< FString > & InFiles, const TMap< FString, FString > & InResults, TMap< FString, FGitSourceControlState > & OutStates );

/**
 * Checks remote branches to see file differences.
 *
 * @param	CurrentBranchName The current branch we are on.
 * @param	InPathToGitBinary	The path to the Git binary
 * @param	InRepositoryRoot	The Git repository from where to run the command - usually the Game directory
 * @param	OnePath				The file to be checked
 * @param	OutErrorMessages	Any errors (from StdErr) as an array per-line
 */
void CheckRemote(const FString& InPathToGitBinary, const FString& InRepositoryRoot, const TArray<FString>& Files,
				 TArray<FString>& OutErrorMessages, TMap<FString, FGitSourceControlState>& OutStates);

/**
 * Run a Git "status" command and parse it.
 *
 * @param	InPathToGitBinary	The path to the Git binary
 * @param	InRepositoryRoot	The Git repository from where to run the command - usually the Game directory (can be empty)
 * @param	InUsingLfsLocking	Tells if using the Git LFS file Locking workflow
 * @param	InFiles				The files to be operated on
 * @param	OutErrorMessages	Any errors (from StdErr) as an array per-line
 * @param   OutStates           The resultant states
 * @returns true if the command succeeded and returned no errors
 */
bool RunUpdateStatus(const FString& InPathToGitBinary, const FString& InRepositoryRoot, const bool InUsingLfsLocking, const TArray<FString>& InFiles,
					 TArray<FString>& OutErrorMessages, TMap<FString, FGitSourceControlState>& OutStates);

#if ENGINE_MAJOR_VERSION == 5
/**
 * Keep Consistency of being file staged
 *
 * @param	Filename			Saved filename
 * @param	Pkg					Package (for adapting delegate)
 * @param   ObjectSaveContext	Context for save (for adapting delegate)
 */
void UpdateFileStagingOnSaved(const FString& Filename, UPackage* Pkg, FObjectPostSaveContext ObjectSaveContext);
#endif

/**
 * Keep Consistency of being file staged with simple argument
 *
 * @param	Filename			Saved filename
 */
bool UpdateFileStagingOnSavedInternal(const FString& Filename);
	
/**
 * 
 *
 * @param	Filename			Saved filename
 * @param	Pkg					Package (for adapting delegate)
 * @param   ObjectSaveContext	Context for save (for adapting delegate)
 */    
void UpdateStateOnAssetRename(const FAssetData& InAssetData, const FString& InOldName);

#if ENGINE_MAJOR_VERSION == 5
/**
 * 
 *
 * @param	Filename			Saved filename
 * @param	Pkg					Package (for adapting delegate)
 * @param   ObjectSaveContext	Context for save (for adapting delegate)
 */
bool UpdateChangelistStateByCommand();
#endif
	
/**
 * Run a Git "cat-file" command to dump the binary content of a revision into a file.
 *
 * @param	InPathToGitBinary	The path to the Git binary
 * @param	InRepositoryRoot	The Git repository from where to run the command - usually the Game directory
 * @param	InParameter			The parameters to the Git show command (rev:path)
 * @param	InDumpFileName		The temporary file to dump the revision
 * @returns true if the command succeeded and returned no errors
*/
bool RunDumpToFile(const FString& InPathToGitBinary, const FString& InRepositoryRoot, const FString& InParameter, const FString& InDumpFileName);

/**
 * Run a Git "log" command and parse it.
 *
 * @param	InPathToGitBinary	The path to the Git binary
 * @param	InRepositoryRoot	The Git repository from where to run the command - usually the Game directory
 * @param	InFile				The file to be operated on
 * @param	bMergeConflict		In case of a merge conflict, we also need to get the tip of the "remote branch" (MERGE_HEAD) before the log of the "current branch" (HEAD)
 * @param	OutErrorMessages	Any errors (from StdErr) as an array per-line
 * @param	OutHistory			The history of the file
 */
bool RunGetHistory(const FString& InPathToGitBinary, const FString& InRepositoryRoot, const FString& InFile, bool bMergeConflict, TArray<FString>& OutErrorMessages, TGitSourceControlHistory& OutHistory);

/**
 * Helper function to convert a filename array to relative paths.
 * @param	InFileNames		The filename array
 * @param	InRelativeTo	Path to the WorkspaceRoot
 * @return an array of filenames, transformed into relative paths
 */
TArray<FString> RelativeFilenames(const TArray<FString>& InFileNames, const FString& InRelativeTo);

/**
 * Helper function to convert a filename array to absolute paths.
 * @param	InFileNames		The filename array (relative paths)
 * @param	InRelativeTo	Path to the WorkspaceRoot
 * @return an array of filenames, transformed into absolute paths
 */
TArray<FString> AbsoluteFilenames(const TArray<FString>& InFileNames, const FString& InRelativeTo);

/**
 * Remove redundant errors (that contain a particular string) and also
 * update the commands success status if all errors were removed.
 */
void RemoveRedundantErrors(FGitSourceControlCommand& InCommand, const FString& InFilter);

	bool RunLFSCommand(const FString& InCommand, const FString& InRepositoryRoot, const FString& GitBinaryFallback, const TArray<FString>& InParameters, const TArray<FString>& InFiles, TArray<FString>& OutResults, TArray<FString>& OutErrorMessages);

/**
 * Helper function for various commands to update cached states.
 * @returns true if any states were updated
 */
GITSOURCECONTROL_API bool UpdateCachedStates( const TMap< const FString, FGitState > & InResults );

/**
* Helper function for various commands to collect new states.
* @returns true if any states were updated
*/
GITSOURCECONTROL_API bool CollectNewStates( const TMap< FString, FGitSourceControlState > & InStates, TMap< const FString, FGitState > & OutResults );
	
/**
 * Helper function for various commands to collect new states.
 * @returns true if any states were updated
 */
bool CollectNewStates(const TArray<FString>& InFiles, TMap<const FString, FGitState>& OutResults, EFileState::Type FileState, ETreeState::Type TreeState = ETreeState::Unset, ELockState::Type LockState = ELockState::Unset, ERemoteState::Type RemoteState = ERemoteState::Unset);

	/**
		 * Run 'git lfs locks" to extract all lock information for all files in the repository
		 *
		 * @param	InRepositoryRoot	The Git repository from where to run the command - usually the Game directory
		 * @param   GitBinaryFallBack   The Git binary fallback path
		 * @param	OutErrorMessages    Any errors (from StdErr) as an array per-line
		 * @param	OutLocks		    The lock results (file, username)
		 * @returns true if the command succeeded and returned no errors
		 */
	bool GetAllLocks(const FString& InRepositoryRoot, const FString& GitBinaryFallBack, TArray<FString>& OutErrorMessages, TMap<FString, FString>& OutLocks, bool bInvalidateCache = false);

/**
 * Gets locks from state cache
 */
void GetLockedFiles(const TArray<FString>& InFiles, TArray<FString>& OutFiles);

/**
 * Checks cache for if this file type is lockable
 */
bool IsFileLFSLockable(const FString& InFile);

/**
 * Gets Git attribute to see if these extensions are lockable
 */
bool CheckLFSLockable(const FString& InPathToGitBinary, const FString& InRepositoryRoot, const TArray<FString>& InFiles, TArray<FString>& OutErrorMessages);

GITSOURCECONTROL_API bool FetchRemote( const FString & InPathToGitBinary, const FString & InPathToRepositoryRoot, bool InUsingGitLfsLocking, TArray< FString > & OutResults, TArray< FString > & OutErrorMessages );

bool PullOrigin(const FString& InPathToGitBinary, const FString& InPathToRepositoryRoot, const TArray<FString>& InFiles, TArray<FString>& OutFiles,
				TArray<FString>& OutResults, TArray<FString>& OutErrorMessages);


GITSOURCECONTROL_API TSharedPtr< class ISourceControlRevision, ESPMode::ThreadSafe > GetOriginRevisionOnBranch( const FString & InPathToGitBinary, const FString & InRepositoryRoot, const FString & InRelativeFileName, TArray< FString > & OutErrorMessages, const FString & BranchName );

}
