from __future__ import annotations

import csv
import datetime
import io
import json
import os
from abc import ABC, abstractmethod
from typing import Any, List, Optional

import gradio as gr
from gradio import encryptor, utils


class FlaggingCallback(ABC):
    @abstractmethod
    def setup(self, flagging_dir: str):
        pass

    @abstractmethod
    def flag(
        self,
        interface: gr.Interface,
        input_data: List[Any],
        output_data: List[Any],
        flag_option: Optional[str] = None,
        flag_index: Optional[int] = None,
        username: Optional[str] = None,
    ) -> int:
        pass


class SimpleCSVLogger(FlaggingCallback):
    def setup(self, flagging_dir: str):
        self.flagging_dir = flagging_dir
        os.makedirs(flagging_dir, exist_ok=True)

    def flag(
        self,
        interface: gr.Interface,
        input_data: List[Any],
        output_data: List[Any],
        flag_option: Optional[str] = None,
        flag_index: Optional[int] = None,
        username: Optional[str] = None,
    ) -> int:
        flagging_dir = self.flagging_dir
        log_filepath = "{}/log.csv".format(flagging_dir)

        csv_data = []
        for i, input in enumerate(interface.input_components):
            csv_data.append(
                input.save_flagged(
                    flagging_dir,
                    interface.config["input_components"][i]["label"],
                    input_data[i],
                    None,
                )
            )
        for i, output in enumerate(interface.output_components):
            csv_data.append(
                output.save_flagged(
                    flagging_dir,
                    interface.config["output_components"][i]["label"],
                    output_data[i],
                    None,
                )
                if output_data[i] is not None
                else ""
            )

        with open(log_filepath, "a", newline="") as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow(utils.santize_for_csv(csv_data))

        with open(log_filepath, "r") as csvfile:
            line_count = len([None for row in csv.reader(csvfile)]) - 1
        return line_count


class CSVLogger(FlaggingCallback):
    def setup(self, flagging_dir: str):
        self.flagging_dir = flagging_dir
        os.makedirs(flagging_dir, exist_ok=True)

    def flag(
        self,
        interface: gr.Interface,
        input_data: List[Any],
        output_data: List[Any],
        flag_option: Optional[str] = None,
        flag_index: Optional[int] = None,
        username: Optional[str] = None,
    ) -> int:
        flagging_dir = self.flagging_dir
        log_fp = "{}/log.csv".format(flagging_dir)
        encryption_key = (
            lambda: interface.encryption_key if interface.encrypt else None
        )()
        is_new = not os.path.exists(log_fp)
        output_only_mode = input_data is None

        if flag_index is None:
            csv_data = []
            if not output_only_mode:
                for i, input in enumerate(interface.input_components):
                    csv_data.append(
                        input.save_flagged(
                            flagging_dir,
                            interface.config["input_components"][i]["label"],
                            input_data[i],
                            encryption_key,
                        )
                    )
            for i, output in enumerate(interface.output_components):
                csv_data.append(
                    output.save_flagged(
                        flagging_dir,
                        interface.config["output_components"][i]["label"],
                        output_data[i],
                        encryption_key,
                    )
                    if output_data[i] is not None
                    else ""
                )
            if not output_only_mode:
                if flag_option is not None:
                    csv_data.append(flag_option)
                if username is not None:
                    csv_data.append(username)
                csv_data.append(str(datetime.datetime.now()))
            if is_new:
                headers = []
                if not output_only_mode:
                    headers += [
                        interface["label"]
                        for interface in interface.config["input_components"]
                    ]
                headers += [
                    interface["label"]
                    for interface in interface.config["output_components"]
                ]
                if not output_only_mode:
                    if interface.flagging_options is not None:
                        headers.append("flag")
                    if username is not None:
                        headers.append("username")
                    headers.append("timestamp")

        def replace_flag_at_index(file_content):
            file_content = io.StringIO(file_content)
            content = list(csv.reader(file_content))
            header = content[0]
            flag_col_index = header.index("flag")
            content[flag_index][flag_col_index] = flag_option
            output = io.StringIO()
            writer = csv.writer(output)
            writer.writerows(utils.santize_for_csv(content))
            return output.getvalue()

        if interface.encrypt:
            output = io.StringIO()
            if not is_new:
                with open(log_fp, "rb") as csvfile:
                    encrypted_csv = csvfile.read()
                    decrypted_csv = encryptor.decrypt(
                        interface.encryption_key, encrypted_csv
                    )
                    file_content = decrypted_csv.decode()
                    if flag_index is not None:
                        file_content = replace_flag_at_index(file_content)
                    output.write(utils.santize_for_csv(file_content))
            writer = csv.writer(output)
            if flag_index is None:
                if is_new:
                    writer.writerow(headers)
                writer.writerow(csv_data)
            with open(log_fp, "wb") as csvfile:
                csvfile.write(
                    utils.santize_for_csv(
                        encryptor.encrypt(
                            interface.encryption_key, output.getvalue().encode()
                        )
                    )
                )
        else:
            if flag_index is None:
                with open(log_fp, "a", newline="") as csvfile:
                    writer = csv.writer(csvfile)
                    if is_new:
                        writer.writerow(utils.santize_for_csv(headers))
                    writer.writerow(utils.santize_for_csv(csv_data))
            else:
                with open(log_fp) as csvfile:
                    file_content = csvfile.read()
                    file_content = replace_flag_at_index(file_content)
                with open(
                    log_fp, "w", newline=""
                ) as csvfile:
                    csvfile.write(utils.santize_for_csv(file_content))
        with open(log_fp, "r") as csvfile:
            line_count = len([None for row in csv.reader(csvfile)]) - 1
        return line_count


class HuggingFaceDatasetSaver(FlaggingCallback):
    def __init__(
        self,
        hf_foken: str,
        dataset_name: str,
        organization: Optional[str] = None,
        private: bool = False,
        verbose: bool = True,
    ):
        self.hf_foken = hf_foken
        self.dataset_name = dataset_name
        self.organization_name = organization
        self.dataset_private = private
        self.verbose = verbose

    def setup(self, flagging_dir: str):
        try:
            import huggingface_hub
        except (ImportError, ModuleNotFoundError):
            raise ImportError(
                "Package `huggingface_hub` not found is needed "
                "for HuggingFaceDatasetSaver. Try 'pip install huggingface_hub'."
            )
        path_to_dataset_repo = huggingface_hub.create_repo(
            name=self.dataset_name,
            token=self.hf_foken,
            private=self.dataset_private,
            repo_type="dataset",
            exist_ok=True,
        )
        self.path_to_dataset_repo = path_to_dataset_repo
        self.flagging_dir = flagging_dir
        self.dataset_dir = os.path.join(flagging_dir, self.dataset_name)
        self.repo = huggingface_hub.Repository(
            local_dir=self.dataset_dir,
            clone_from=path_to_dataset_repo,
            use_auth_token=self.hf_foken,
        )
        self.repo.git_pull()

        self.log_file = os.path.join(self.dataset_dir, "data.csv")
        self.infos_file = os.path.join(self.dataset_dir, "dataset_infos.json")

    def flag(
        self,
        interface: gr.Interface,
        input_data: List[Any],
        output_data: List[Any],
        flag_option: Optional[str] = None,
        flag_index: Optional[int] = None,
        username: Optional[str] = None,
    ) -> int:
        is_new = not os.path.exists(self.log_file)
        infos = {"flagged": {"features": {}}}

        with open(self.log_file, "a", newline="") as csvfile:
            writer = csv.writer(csvfile)

            file_preview_types = {
                gr.inputs.Audio: "Audio",
                gr.outputs.Audio: "Audio",
                gr.inputs.Image: "Image",
                gr.outputs.Image: "Image",
            }

            if is_new:
                headers = []

                for i, component in enumerate(interface.input_components):
                    component_label = interface.config["input_components"][i][
                        "label"
                    ] or "Input_{}".format(i)
                    headers.append(component_label)
                    infos["flagged"]["features"][component_label] = {
                        "dtype": "string",
                        "_type": "Value",
                    }
                    if isinstance(component, tuple(file_preview_types)):
                        headers.append(component_label + " file")
                        for _component, _type in file_preview_types.items():
                            if isinstance(component, _component):
                                infos["flagged"]["features"][
                                    component_label + " file"
                                ] = {"_type": _type}
                                break

                for i, component in enumerate(interface.output_components):
                    component_label = interface.config["output_components"][i][
                        "label"
                    ] or "Output_{}".format(i)
                    headers.append(component_label)
                    infos["flagged"]["features"][component_label] = {
                        "dtype": "string",
                        "_type": "Value",
                    }
                    if isinstance(component, tuple(file_preview_types)):
                        headers.append(component_label + " file")
                        for _component, _type in file_preview_types.items():
                            if isinstance(component, _component):
                                infos["flagged"]["features"][
                                    component_label + " file"
                                ] = {"_type": _type}
                                break

                if interface.flagging_options is not None:
                    headers.append("flag")
                    infos["flagged"]["features"]["flag"] = {
                        "dtype": "string",
                        "_type": "Value",
                    }

                writer.writerow(utils.santize_for_csv(headers))

            csv_data = []
            for i, component in enumerate(interface.input_components):
                label = interface.config["input_components"][i][
                    "label"
                ] or "Input_{}".format(i)
                filepath = component.save_flagged(
                    self.dataset_dir, label, input_data[i], None
                )
                csv_data.append(filepath)
                if isinstance(component, tuple(file_preview_types)):
                    csv_data.append(
                        "{}/resolve/main/{}".format(self.path_to_dataset_repo, filepath)
                    )
            for i, component in enumerate(interface.output_components):
                label = interface.config["output_components"][i][
                    "label"
                ] or "Output_{}".format(i)
                filepath = (
                    component.save_flagged(
                        self.dataset_dir, label, output_data[i], None
                    )
                    if output_data[i] is not None
                    else ""
                )
                csv_data.append(filepath)
                if isinstance(component, tuple(file_preview_types)):
                    csv_data.append(
                        "{}/resolve/main/{}".format(self.path_to_dataset_repo, filepath)
                    )
            if flag_option is not None:
                csv_data.append(flag_option)

            writer.writerow(utils.santize_for_csv(csv_data))

        if is_new:
            json.dump(infos, open(self.infos_file, "w"))

        with open(self.log_file, "r") as csvfile:
            line_count = len([None for row in csv.reader(csvfile)]) - 1

        self.repo.push_to_hub(commit_message="Flagged sample #{}".format(line_count))

        return line_count