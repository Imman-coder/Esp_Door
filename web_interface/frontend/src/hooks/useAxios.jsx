import Axios from "axios";

const axios = Axios.create({
  baseURL: import.meta.env.VITE_BACKEND_URL || window.location.href,
});

export default axios;
