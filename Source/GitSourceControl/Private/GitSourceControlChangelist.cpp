#if ENGINE_MAJOR_VERSION == 5
#include "GitSourceControlChangelist.h"

FGitSourceControlChangelist FGitSourceControlChangelist::WorkingChangelist(TEXT("Working"), true);
FGitSourceControlChangelist FGitSourceControlChangelist::StagedChangelist(TEXT("Staged"), true);
#endif
