// Helper function to log messages to a file
function logMessage(message) {
  const folder = DriveApp.getRootFolder();  // Use the root folder directly
  let logFile;

  // Check if the log file already exists
  const files = folder.getFilesByName('script_log.txt');
  
  if (files.hasNext()) {
    logFile = files.next();  // Use existing file if it exists
  } else {
    // Create a new log file if none exists
    logFile = folder.createFile('script_log.txt', '', MimeType.PLAIN_TEXT);
  }

  // Read the current content of the file
  const currentContent = logFile.getBlob().getDataAsString();

  // Append the new log message to the current content
  const timestamp = new Date().toISOString();
  const newContent = currentContent + `(${timestamp}) ${message}\n`;

  // Write the new content back to the file
  // logFile.setContent(newContent);
}

function ensureFolderExists(OWNER, REPO, folderPath, GITHUB_TOKEN) {
    const gitkeepPath = `${folderPath}/.gitkeep`;
    
    logMessage("ensuring folder existence");
    try {
        const fileCheckResponse = UrlFetchApp.fetch(`https://api.github.com/repos/${OWNER}/${REPO}/contents/${gitkeepPath}`, {
            method: 'GET',
            headers: {
                Authorization: `Bearer ${GITHUB_TOKEN}`,
                Accept: 'application/vnd.github.v3+json'
            }
        });

        if (fileCheckResponse.getResponseCode() === 200) {
            return; // Folder already exists
        }
    } catch (e) {
        // File doesn't exist, so create it
        const payload = {
            message: `Create folder ${folderPath}`,
            content: Utilities.base64Encode(""), // Empty file
            branch: "main"
        };

        UrlFetchApp.fetch(`https://api.github.com/repos/${OWNER}/${REPO}/contents/${gitkeepPath}`, {
            method: 'PUT',
            contentType: 'application/json',
            headers: {
                Authorization: `Bearer ${GITHUB_TOKEN}`,
                Accept: 'application/vnd.github.v3+json'
            },
            payload: JSON.stringify(payload)
        });
    }
}


function test(){
  logMessage("test");
}

function doPost(e) {
  // Retrieve camera main information 
  const CAMNAME = e.parameters.sourceCam || 'ESP32-CAM_unknown';
  const REQUESTTYPE = e.parameters.requestType || 'upload';

  // GitHub Configuration;
  const REPO = 'camfleet';
  const TARGET_FOLDER = CAMNAME + '/';

  // Get parameters and define image properties
  const jpgName = Utilities.formatDate(new Date(), 'GMT+1', 'yyyyMMdd-HHmmss') + '_' + CAMNAME + '.jpg';
  const imagePath = TARGET_FOLDER + jpgName;


  logMessage(`CAMNAME: ${CAMNAME}`);
  logMessage(`REQUESTTYPE: ${REQUESTTYPE}`);
  logMessage(`REPO: ${REPO}`);
  logMessage(`TARGET_FOLDER: ${TARGET_FOLDER}`);
  logMessage(`jpgName: ${jpgName}`);
  logMessage(`imagePath: ${imagePath}`);
  
  // Retrieve secure GitHub token from PropertiesService
  const GITHUB_TOKEN = PropertiesService.getScriptProperties().getProperty('GITHUB_TOKEN');
  const OWNER = PropertiesService.getScriptProperties().getProperty('OWNER');
  logMessage(`Private Properties assessed`);

  // Ensure the request is for upload
  if (REQUESTTYPE == 'upload') {
    
    logMessage(`enter requestType == upload IF`);
    try {
      // create camera subfolder if it doesnt exist
      ensureFolderExists(OWNER, REPO, CAMNAME, GITHUB_TOKEN)
      // Decode and prepare the image blob
      const data = Utilities.base64Decode(e.postData.contents);
      const blob = Utilities.newBlob(data, 'image/jpg', jpgName);
      const imageBase64 = Utilities.base64Encode(blob.getBytes());
      logMessage(`Image is decoded.`);
      // // Check if the file already exists (to get SHA for overwriting)
      // const existingFileResponse = UrlFetchApp.fetch(`https://api.github.com/repos/${OWNER}/${REPO}/contents/${imagePath}`, {
      //   method: 'GET',
      //   headers: {
      //     Authorization: `Bearer ${GITHUB_TOKEN}`,
      //     Accept: 'application/vnd.github.v3+json'
      //   }
      // });

      // let sha = null;
      // if (existingFileResponse.getResponseCode() === 200) {
      //   const existingFile = JSON.parse(existingFileResponse.getContentText());
      //   sha = existingFile.sha; // Capture SHA for overwriting
      // }

      // Upload or update the image to GitHub
      const uploadPayload = {
        message: `Upload image from ${CAMNAME}`,
        content: imageBase64,
        // sha: sha // Include SHA if file exists (for update)
      };

      const uploadResponse = UrlFetchApp.fetch(`https://api.github.com/repos/${OWNER}/${REPO}/contents/${imagePath}`, {
        method: 'PUT',
        contentType: 'application/json',
        headers: {
          Authorization: `Bearer ${GITHUB_TOKEN}`,
          Accept: 'application/vnd.github.v3+json'
        },
        payload: JSON.stringify(uploadPayload)
      });
      const responseText = uploadResponse.getContentText();
      logMessage(responseText);

      if (uploadResponse.getResponseCode() === 201 || uploadResponse.getResponseCode() === 200) {
        logMessage(`✅ Image uploaded successfully from ${CAMNAME}`);
        return ContentService.createTextOutput('✅ Image uploaded successfully!');
      } else {
        const errorMessage = `GitHub API Error: ${uploadResponse.getContentText()}`;
        logMessage(`❌ Upload failed: ${errorMessage}`);
        throw new Error(errorMessage);
      }

    } catch (error) {
      logMessage(`❌ Upload Error: ${error.message}`);
      console.error('❌ Upload Error:', error);
      return ContentService.createTextOutput('❌ Upload failed. Check logs for details.');
    }
  }
}

function deleteOlder48h() {
  const GITHUB_TOKEN = PropertiesService.getScriptProperties().getProperty('GITHUB_TOKEN');
  const OWNER = PropertiesService.getScriptProperties().getProperty('OWNER');
  const REPO = 'camfleet';

  try {
    const folderUrl = `https://api.github.com/repos/${OWNER}/${REPO}/contents/`;
    const foldersResponse = UrlFetchApp.fetch(folderUrl, {
      headers: {
        Authorization: `Bearer ${GITHUB_TOKEN}`,
        Accept: 'application/vnd.github.v3+json',
      },
    });

    if (foldersResponse.getResponseCode() !== 200) throw new Error('Failed to fetch folder list.');
    const folders = JSON.parse(foldersResponse.getContentText());

    // Iterate over all folders (camera folders)
    folders.forEach((folder) => {
      if (folder.type == 'dir') {
        cleanFolder(OWNER, REPO, folder.path, GITHUB_TOKEN);
      }
    });
  } catch (error) {
    logMessage(`❌ Error during cleanup process: ${error.message}`);
    console.error('❌ Error during cleanup process:', error);
  }
}

// Helper function to clean a specific folder
function cleanFolder(owner, repo, folderPath, token) {
  try {
    const folderUrl = `https://api.github.com/repos/${owner}/${repo}/contents/${folderPath}`;
    const response = UrlFetchApp.fetch(folderUrl, {
      headers: {
        Authorization: `Bearer ${token}`,
        Accept: 'application/vnd.github.v3+json',
      },
    });

    if (response.getResponseCode() !== 200) throw new Error(`Failed to list folder: ${folderPath}`);
    const files = JSON.parse(response.getContentText());
    const now = new Date();

    files.forEach((file) => {
      if (file.type === 'file' && !file.name.endsWith(".gitkeep")) {
        // Check age of each file (commit date retrieval)
        const commitUrl = `https://api.github.com/repos/${owner}/${repo}/commits?path=${file.path}`;
        const commitResponse = UrlFetchApp.fetch(commitUrl, {
          headers: {
            Authorization: `Bearer ${token}`,
            Accept: 'application/vnd.github.v3+json',
          },
        });

        if (commitResponse.getResponseCode() !== 200) return;

        const commitData = JSON.parse(commitResponse.getContentText());
        if (commitData.length === 0) return;

        const lastModified = new Date(commitData[0].commit.committer.date);
        const ageMinutes = (now - lastModified) / (1000 * 60);

        // Delete if older than 48 hours (2880 minutes)
        if (ageMinutes > 2880) {
          logMessage(`Deleting old image: ${file.path}`);
          console.log(`Deleting old image: ${file.path}`);

          UrlFetchApp.fetch(file.url, {
            method: 'DELETE',
            headers: {
              Authorization: `Bearer ${token}`,
              Accept: 'application/vnd.github.v3+json',
            },
            payload: JSON.stringify({
              message: `Delete old image: ${file.path}`,
              sha: file.sha,
            }),
          });
        }
      }
    });
  } catch (error) {
    logMessage(`❌ Error cleaning folder ${folderPath}: ${error.message}`);
    console.error(`❌ Error cleaning folder ${folderPath}:`, error);
  }
}

// Schedule cleanup task (run daily)
function setupTrigger() {
  ScriptApp.newTrigger('deleteOlder48h').timeBased().everyDays(1).create();
}
