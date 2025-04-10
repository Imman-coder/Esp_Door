import axios from "axios";
import React, { useEffect, useState } from "react";
import Dropzone, { useDropzone } from "react-dropzone";
// import md5File from "md5-file";

function UpdatePage() {
  const [file, setFile] = useState(null);
  const [progress, setProgress] = useState(0);
  const [uploading, setUploading] = useState(false);
  const [msg, setMsg] = useState("");
  const [mode, setMode] = useState("fs");

  const handleChange = (file) => {
    setFile(file[0]);
  };

  const { getRootProps, getInputProps } = useDropzone({
    accept: { "application/x-binary": [".bin"] },
    maxFiles: 1,
    onDrop: handleChange,
  });

  useEffect(() => {
    if (file && file.size > 0) {
      setMsg("File selected: " + file.name);
    } else {
      setMsg("Drag 'n' drop some files here, or click to select files");
    }
  }, [uploading, file]);

  const handleUpload = (e) => {
    e.preventDefault();
    const formData = new FormData();
    formData.append("update", file);
    setUploading(true);

    axios
      .post("/update", formData, {
        headers: {
          "Content-Type": "multipart/form-data",
          "Access-Control-Allow-Origin": "*",
          "Access-Control-Allow-Headers": "*",
          params: {
            type: {
              mode,
              // hash: hash,
            },
          },
        },
        withCredentials: false,
        onUploadProgress: (progressEvent) => {
          const { loaded, total } = progressEvent;
          const percent = Math.floor((loaded * 100) / total);
          setProgress(percent);
        },
      })
      .then((res) => {
        console.log(res.data);
        setUploading(false);
        if (res.data.status === "success") {
          alert("Firmware uploaded successfully!");
        } else {
          alert("Firmware upload failed!");
        }
      })
      .catch((err) => {
        console.error(err);
        setUploading(false);
        alert("Error uploading firmware!");
      });
  };
  return (
    <div className=" p-6 w-full">
      <div className="card bg-base-200 shadow-sm ">
        <div className="card-body w-full items-center">
          <span className="mb-8 text-3xl bold">
            Upload the firmware for esp_door project.
          </span>
          <div
            {...getRootProps({ className: "dropzone" })}
            className="border-2 border-dashed p-4 rounded-lg w-[95%] h-62 flex justify-center content-center flex-wrap cursor-pointer"
          >
            <input {...getInputProps()} />
            {uploading ? (
              <div className="flex flex-col items-center w-full justify-center">
                <div
                  className="radial-progress text-primary"
                  style={{ "--value": progress }}
                  aria-valuenow={progress}
                  role="progressbar"
                >
                  {progress}%
                </div>
                <p className="mt-4 ">Uploading...</p>
              </div>
              
            ) : (
              <p className="grow-[inherit]">{msg}</p>
            )}
          </div>
          <div className="flex flex-col mt-8 w-full">
            <fieldset className="fieldset">
              <legend className="fieldset-legend">OTA Mode</legend>
              <select
                onChange={(e) => {
                  console.log(e.target.value);
                  setMode(e.target.value);
                }}
                defaultValue="Firmware"
                className="select"
              >
                <option value="firm">Firmware</option>
                <option value="fs">Filesystem</option>
              </select>
            </fieldset>
            <div
              className="btn btn-primary mt-2"
              onClick={handleUpload}
              disabled={!file || uploading}
            >
              Upload
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}

export default UpdatePage;
