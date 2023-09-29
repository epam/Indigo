import axios from 'axios';
import {Endpoint_Values} from "../components/search-type/constants/types";

const libs = 'libraries/libraries';
const search = 'libraries/search';
const render = 'indigo/render';

const config = {
  headers: {
    'Content-Type': 'application/json',
  },
};

export default class ApiService {

  static async getLibraries() {
    return await axios.get(`${localStorage.getItem("endpoint")}/${libs}`);
  }

  static async getLibraryById(id) {
    return await axios.get(`${localStorage.getItem("endpoint")}/${libs}/${id}`);
  }

  static async search(params) {
    return await axios.post(`${localStorage.getItem("endpoint")}/${search}`, params, config);
  }

  static async searchById(id) {
    return await axios.get(`${localStorage.getItem("endpoint")}/${search}/${id}`);
  }

  static async addLibrary(params) {
    if (!params.name) {
      throw new Error('Library name cannot be empty.');
    }
    return await axios.post(`${localStorage.getItem("endpoint")}/${libs}`, params, config);
  }

  //TODO add possibility to upload library from S3 bucket
  static uploadToLibrary(params, id) {
    const headers = {
      headers: {
        'Content-Type': 'chemical/x-mdl-sdfile',
      },
    };
    return axios.post(`${localStorage.getItem("endpoint")}/libraries/libraries/${id}/uploads`, params, headers);
  }

  static async deleteLibrary(params) {
    return await axios.delete(`${localStorage.getItem("endpoint")}/libraries/libraries/${params}`, params, config);
  }

  static async render(params) {
    return await axios.post(`${Endpoint_Values.Postgres}/${render}`, params, config);
  }

  static async recount(id) {
    return await axios.get(`${localStorage.getItem("endpoint")}/libraries/${id}/recount`);
  }
}
