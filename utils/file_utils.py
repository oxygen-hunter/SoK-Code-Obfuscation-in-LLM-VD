from pathlib import Path
from utils.constants import Language, DatasetName

class PathManager:
    def __init__(self, base_dir="data"):
        self.base_dir = Path(base_dir)

    def get_original_code_path(self, language: Language, dataset: DatasetName, filename: str) -> Path:
        return self.base_dir / "original" / language.value / dataset.value / filename

    def get_obfuscated_code_path(self, language: Language, dataset: DatasetName,
                                  root_class: str, mid_class: str, sub_class: str,
                                  filename: str) -> Path:
        return self.base_dir / "obfuscated" / language.value / dataset.value / root_class / mid_class / sub_class / filename

    def get_detection_report_path(self, language: Language, dataset: DatasetName,
                                  root_class: str, mid_class: str, sub_class: str,
                                  filename: str) -> Path:
        return self.base_dir / "reports" / language.value / dataset.value / root_class / mid_class / sub_class / filename

    def ensure_dirs(self, language: Language, dataset: DatasetName,
                    root_class: str, mid_class: str, sub_class: str):
        (self.base_dir / "original" / language.value / dataset.value).mkdir(parents=True, exist_ok=True)
        (self.base_dir / "obfuscated" / language.value / dataset.value / root_class / mid_class / sub_class).mkdir(parents=True, exist_ok=True)
        (self.base_dir / "reports" / language.value / dataset.value / root_class / mid_class / sub_class).mkdir(parents=True, exist_ok=True)

    def generate_sample_name(self, index: int, sub_class: str, ext: str) -> str:
        """
        示例: sample_003_gradient_based.sol
        """
        return f"sample_{index:03d}_{sub_class.replace('-', '_')}.{ext}"
