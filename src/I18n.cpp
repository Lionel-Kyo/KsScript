#include "pch.h"
#include "I18n.h"
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <sstream>

static Language s_currentLanguage = Language::English;
static bool s_initialized = false;

static const char* s_languageNames[static_cast<size_t>(Language::Count)] = {
    "English",
    "廣東話",
    "繁體中文",
    "简体中文",
    "日本語",
    "한국어",
    "Español",
    "Português",
    "Français",
    "Deutsch"
};

// Language codes for config persistence
static const char* s_languageCodes[static_cast<size_t>(Language::Count)] = {
    "en",
    "yue-HK",
    "zh-TW",
    "zh-CN",
    "ja",
    "ko",
    "es",
    "pt",
    "fr",
    "de"
};

// [Language][StringId]
static const char* s_translations[static_cast<size_t>(Language::Count)][static_cast<size_t>(StringId::Count)] = {
    // English
    {
        "KsScript",
        "Script Manager",
        "Open Scripts",
        "Script Recorder",
        "Functions Reference",
        "Keys Reference",
        "Enable",
        "Script Name",
        "Execution Mode",
        "Start Key",
        "End Key",
        "Disable",
        "Enable",
        "Stop Test Run",
        "Run Test",
        "Remove",
        "Reload",
        "Open Script Location",
        "Change keys and mode",
        "Enable All",
        "Disable All",
        "Remove All",
        "Save",
        "Cancel",
        "OK",
        "StartKey:",
        "EndKey:",
        "RunMode:",
        "Record",
        "Stop Rec",
        "Play",
        "Stop Play",
        "Save Script",
        "Status: RECORDING...",
        "Status: EXECUTING...",
        "Status: IDLE",
        "Name",
        "Parameters",
        "Explanation",
        "Value",
        "File Error",
        "Script Syntax Error",
        "Save Failed",
        "Save Success",
        "Failed to read UTF-8 script file.\r\n{}",
        "Failed to read file for modification",
        "Failed to open file for writing",
        "An unexpected error occurred while writing to file",
        "Script settings updated: {}",
        "Script is saved to: {}",
        "Current Coordinate",
        // Parser errors
        "Missing or invalid parentheses '()'",
        "Trailing characters found after closing parenthesis ')', If needed, you can add text after #.",
        "Invalid hotkey format: ",
        "Invalid run mode parameter: ",
        "Invalid delay parameter (must be an integer): ",
        "Invalid mouse button parameter: ",
        "Invalid coordinate parameter (must be integers): ",
        "Move command expects exactly 2 parameters (x, y): ",
        "Invalid key parameter or virtual key code: ",
        "Wheel command does not accept parameters: ",
        "Invalid string format (expected quoted string): ",
        "Invalid string format (unclosed quote): ",
        "Invalid parameter format: ",
        "ShowCoordinate command does not accept parameters: ",
        "An unexpected exception occurred during line parsing.",
        "Unknown function: ",
        "Syntax Error\r\n{}\r\nLine {}:\r\n\"{}\"",
        "Syntax Error\r\nStart Key already defined\r\nLine {}:\r\n\"{}\"",
        "Syntax Error\r\nEnd Key already defined\r\nLine {}:\r\n\"{}\"",
        "Syntax Error\r\nRun Mode already defined\r\nLine {}:\r\n\"{}\"",
        // Modes
        "FullSingle",
        "FullContinuous",
        "FullSwitch",
        "Single",
        "Continuous",
        "Switch"
    },
    // Cantonese (Hong Kong) (廣東話)
    {
        "KsScript",
        "腳本管理器",
        "讀取腳本",
        "腳本記錄器",
        "腳本指令說明",
        "鍵盤按鍵說明",
        "啟用",
        "腳本名稱",
        "運行模式",
        "開始按鍵",
        "停止按鍵",
        "關閉腳本",
        "開啟腳本",
        "停止測試執行",
        "測試執行",
        "移除腳本",
        "重新讀取",
        "打開腳本位置",
        "更改按鍵及模式",
        "開啟全部腳本",
        "關閉全部腳本",
        "移除全部腳本",
        "儲存",
        "取消",
        "確定",
        "開始按鍵:",
        "停止按鍵:",
        "運行模式:",
        "錄製",
        "停止錄製",
        "執行",
        "停止執行",
        "儲存腳本",
        "狀態: 錄緊...",
        "狀態: 行緊...",
        "狀態: 無",
        "名稱",
        "引數",
        "說明",
        "數值",
        "檔案錯誤",
        "腳本錯誤",
        "儲存失敗",
        "儲存成功",
        "讀取UTF-8 .txt腳本檔案失敗\r\n{}",
        "無法讀取檔案進行修改",
        "無法開啟檔案寫入",
        "寫入檔案嘅時候時發生非預期錯誤",
        "腳本設定已更新: {}",
        "腳本已儲存到: {}",
        "現時座標",
        // Parser errors
        "缺少或無效嘅括號 '()'",
        "括號 ')' 後面唔應該有其他文字，有需要可以加喺 # 後面;",
        "無效嘅按鍵格式: ",
        "無效嘅運行模式引數: ",
        "無效嘅延遲引數 (必須係整數): ",
        "無效嘅滑鼠掣引數: ",
        "無效嘅座標引數 (必須係整數): ",
        "移動指令只需要2個引數 (x, y): ",
        "無效嘅按鍵名稱或按鍵編號: ",
        "滾輪指令唔接受引數: ",
        "無效嘅文字格式（文字應該用\"\"或''包住）: ",
        "無效嘅文字格式(未閉合嘅引號): ",
        "無效嘅引數格式: ",
        "顯示座標指令唔接受引數: ",
        "解析嗰行嗰陣發生未知例外狀況。",
        "未知嘅指令: ",
        "腳本錯誤\r\n{}\r\n行數: {}:\r\n\"{}\"",
        "腳本錯誤\r\n已經設定咗開始按鍵\r\n行數: {}:\r\n\"{}\"",
        "腳本錯誤，行數: {}:\r\n\"{}\"\r\n已經設定咗停止按鍵",
        "腳本錯誤，行數: {}:\r\n\"{}\"\r\n已經設定咗運行模式",
        // Modes
        "完整單次",
        "完整連續",
        "完整切換",
        "單次",
        "連續",
        "切換"
    },
    // Traditional Chinese (繁體中文)
    {
        "KsScript",
        "腳本管理器",
        "讀取腳本",
        "腳本記錄器",
        "腳本指令說明",
        "鍵盤按鍵說明",
        "開啟",
        "腳本名稱",
        "運行模式",
        "開始按鍵",
        "停止按鍵",
        "關閉腳本",
        "開啟腳本",
        "停止測試執行",
        "測試執行",
        "移除腳本",
        "重新讀取",
        "打開腳本位置",
        "更改按鍵與模式",
        "開啟全部腳本",
        "關閉全部腳本",
        "移除全部腳本",
        "保存",
        "取消",
        "確定",
        "開始按鍵:",
        "停止按鍵:",
        "運行模式:",
        "記錄",
        "停止記錄",
        "執行",
        "停止執行",
        "保存腳本",
        "狀態: 記錄中...",
        "狀態: 執行中...",
        "狀態: 無",
        "名稱",
        "引數",
        "說明",
        "數值",
        "文件錯誤",
        "腳本錯誤",
        "保存失敗",
        "保存成功",
        "讀取UTF-8 .txt腳本文件失敗\r\n{}",
        "無法讀取檔案以進行修改",
        "無法開啟檔案以進行寫入",
        "寫入檔案時發生未預期的錯誤",
        "腳本設定已更新: {}",
        "腳本已保存至: {}",
        "現在座標",
        // Parser errors
        "缺少或無效的括號 '()'",
        "括號 ')' 後面不應該有其他文字，如有需要可以將文字加在 # 之後;",
        "無效的按鍵格式: ",
        "無效的運行模式引數: ",
        "無效的延遲引數 (必須為整數): ",
        "無效的滑鼠按鍵引數: ",
        "無效的座標引數 (必須為整數): ",
        "移動指令只需要2個引數 (x, y): ",
        "無效的按鍵名稱或按鍵編號: ",
        "滾輪指令不接收引數: ",
        "無效的文字格式（文字應該被\"\"或''包圍）: ",
        "無效的文字格式(未閉合的引號): ",
        "無效的引數格式: ",
        "顯示座標指令不接收引數: ",
        "解析該行時發生未知的例外狀況。",
        "未知的指令: ",
        "腳本錯誤\r\n{}\r\n行數: {}:\r\n\"{}\"",
        "腳本錯誤\r\n已經設定了開始按鍵\r\n行數: {}:\r\n\"{}\"",
        "腳本錯誤，行數: {}:\r\n\"{}\"\r\n已經設定了停止按鍵",
        "腳本錯誤，行數: {}:\r\n\"{}\"\r\n已經設定了運行模式",
        // Modes
        "完整單次",
        "完整連續",
        "完整切換",
        "單次",
        "連續",
        "切換"
    },
    // Simplified Chinese (简体中文)
    {
        "KsScript",
        "脚本管理器",
        "打开脚本",
        "脚本记录器",
        "脚本指令说明",
        "键盘按键说明",
        "启用",
        "脚本名称",
        "运行模式",
        "开始按键",
        "停止按键",
        "禁用脚本",
        "启用脚本",
        "停止测试运行",
        "测试运行",
        "移除脚本",
        "重新加载",
        "打开脚本位置",
        "更改按键与模式",
        "启用全部脚本",
        "禁用全部脚本",
        "移除全部脚本",
        "保存",
        "取消",
        "确定",
        "开始按键:",
        "停止按键:",
        "运行模式:",
        "录制",
        "停止录制",
        "运行",
        "停止运行",
        "保存脚本",
        "状态: 录制中...",
        "状态: 运行中...",
        "状态: 空闲",
        "名称",
        "参数",
        "说明",
        "数值",
        "文件错误",
        "脚本错误",
        "保存失败",
        "保存成功",
        "读取UTF-8 .txt脚本文件失败\r\n{}",
        "无法读取文件以进行修改",
        "无法打开文件以进行写入",
        "写入文件时发生意外错误",
        "脚本设置已更新: {}",
        "脚本已保存至: {}",
        "当前坐标",
        // Parser errors
        "缺少或无效的括号 '()'",
        "括号 ')' 后面不应该有其他文字，如有需要可以将文字加在 # 之后;",
        "无效的按键格式: ",
        "无效的运行模式参数: ",
        "无效的延迟参数 (必须为整数): ",
        "无效的鼠标按键参数: ",
        "无效的坐标参数 (必须为整数): ",
        "移动指令只需要2个参数 (x, y): ",
        "无效的按键名称或按键编号: ",
        "滚轮指令不接收参数: ",
        "无效的文本格式（文本应该被\"\"或''包围）: ",
        "无效的文本格式(未闭合的引号): ",
        "无效的参数格式: ",
        "显示坐标指令不接收参数: ",
        "解析该行时发生未知的异常。",
        "未知的指令: ",
        "脚本错误\r\n{}\r\n行数: {}:\r\n\"{}\"",
        "脚本错误\r\n已经设置了开始按键\r\n行数: {}:\r\n\"{}\"",
        "脚本错误，行数: {}:\r\n\"{}\"\r\n已经设置了停止按键",
        "脚本错误，行数: {}:\r\n\"{}\"\r\n已经设置了运行模式",
        // Modes
        "完整单次",
        "完整连续",
        "完整切换",
        "单次",
        "连续",
        "切换"
    },
    // Japanese (日本語)
    {
        "KsScript",
        "スクリプトマネージャー",
        "スクリプトを開く",
        "スクリプトレコーダー",
        "コマンドリファレンス",
        "キーリファレンス",
        "有効",
        "スクリプト名",
        "実行モード",
        "開始キー",
        "停止キー",
        "スクリプトを無効化",
        "スクリプトを有効化",
        "テスト実行停止",
        "テスト実行",
        "スクリプトを削除",
        "再読み込み",
        "スクリプトの場所を開く",
        "キーとモードの変更",
        "すべて有効",
        "すべて無効",
        "すべて削除",
        "保存",
        "キャンセル",
        "OK",
        "開始キー:",
        "停止キー:",
        "実行モード:",
        "録画",
        "録画停止",
        "実行",
        "実行停止",
        "スクリプトを保存",
        "状態: 録画中...",
        "状態: 実行中...",
        "状態: アイドル",
        "名前",
        "引数",
        "説明",
        "値",
        "ファイルエラー",
        "スクリプトエラー",
        "保存失敗",
        "保存成功",
        "UTF-8 .txtスクリプトファイルの読み込みに失敗しました。\r\n{}",
        "変更用のファイルを読み込めませんでした",
        "書き込み用のファイルを開けませんでした",
        "ファイルへの書き込み中に予期しないエラーが発生しました",
        "スクリプト設定が更新されました: {}",
        "スクリプトが保存されました: {}",
        "現在座標",
        // Parser errors
        "括弧 '()' が見つからないか無効です",
        "閉じ括弧 ')' の後に不正な文字があります。注釈は # の後に記述してください。",
        "無効なホットキー形式: ",
        "無効な実行モード引数: ",
        "無効な遅延引数 (整数である必要があります): ",
        "無効なマウスボタン引数: ",
        "無効な座標引数 (整数である必要があります): ",
        "移動コマンドにはちょうど2つの引数 (x, y) が必要です: ",
        "無効なキー名または仮想キーコード: ",
        "ホイールコマンドは引数を受け取りません: ",
        "無効な文字列形式 (\"\" または '' で囲む必要があります): ",
        "無効な文字列形式 (引用符が閉じられていません): ",
        "無効な引数形式: ",
        "座標表示コマンドは引数を受け取りません: ",
        "行の解析中に予期しない例外が発生しました。",
        "未知のコマンド: ",
        "構文エラー\r\n{}\r\n行 {}:\r\n\"{}\"",
        "構文エラー\r\n開始キーは既に定義されています\r\n行 {}:\r\n\"{}\"",
        "構文エラー\r\n停止キーは既に定義されています\r\n行 {}:\r\n\"{}\"",
        "構文エラー\r\n実行モードは既に定義されています\r\n行 {}:\r\n\"{}\"",
        // Modes
        "完全単発",
        "完全連続",
        "完全トグル",
        "単発",
        "連続",
        "トグル"
    },
    // Korean (한국어)
    {
        "KsScript",
        "스크립트 관리자",
        "스크립트 열기",
        "스크립트 레코더",
        "명령어 참조",
        "키 참조",
        "활성화",
        "스크립트 이름",
        "실행 모드",
        "시작 키",
        "종료 키",
        "스크립트 비활성화",
        "스크립트 활성화",
        "테스트 실행 중지",
        "테스트 실행",
        "스크립트 제거",
        "다시 로드",
        "스크립트 위치 열기",
        "키 및 모드 변경",
        "모두 활성화",
        "모두 비활성화",
        "모두 제거",
        "저장",
        "취소",
        "확인",
        "시작 키:",
        "종료 키:",
        "실행 모드:",
        "기록",
        "기록 중지",
        "실행",
        "실행 중지",
        "스크립트 저장",
        "상태: 기록 중...",
        "상태: 실행 중...",
        "상태: 대기",
        "이름",
        "매개변수",
        "설명",
        "값",
        "파일 오류",
        "스크립트 구문 오류",
        "저장 실패",
        "저장 성공",
        "UTF-8 .txt 스크립트 파일을 읽지 못했습니다.\r\n{}",
        "수정을 위해 파일을 읽을 수 없습니다",
        "쓰기를 위해 파일을 열 수 없습니다",
        "파일에 쓰는 중 예기치 않은 오류가 발생했습니다",
        "스크립트 설정이 업데이트되었습니다: {}",
        "스크립트가 저장되었습니다: {}",
        "현재 좌표",
        // Parser errors
        "괄호 '()'가 없거나 잘못되었습니다",
        "닫는 괄호 ')' 뒤에 문자가 있습니다. 필요하면 # 뒤에 텍스트를 추가하세요.",
        "잘못된 핫키 형식: ",
        "잘못된 실행 모드 매개변수: ",
        "잘못된 지연 매개변수 (정수여야 함): ",
        "잘못된 마우스 버튼 매개변수: ",
        "잘못된 좌표 매개변수 (정수여야 함): ",
        "이동 명령에는 정확히 2개의 매개변수 (x, y)가 필요합니다: ",
        "잘못된 키 이름 또는 가상 키 코드: ",
        "휠 명령은 매개변수를 받지 않습니다: ",
        "잘못된 문자열 형식 (\"\" 또는 ''로 감싸야 함): ",
        "잘못된 문자열 형식 (따옴표가 닫히지 않음): ",
        "잘못된 매개변수 형식: ",
        "좌표 표시 명령은 매개변수를 받지 않습니다: ",
        "줄을 구문 분석하는 동안 예기치 않은 예외가 발생했습니다.",
        "알 수 없는 명령어: ",
        "구문 오류\r\n{}\r\n{}번째 줄:\r\n\"{}\"",
        "구문 오류\r\n시작 키가 이미 정의되었습니다\r\n{}번째 줄:\r\n\"{}\"",
        "구문 오류\r\n종료 키가 이미 정의되었습니다\r\n{}번째 줄:\r\n\"{}\"",
        "구문 오류\r\n실행 모드가 이미 정의되었습니다\r\n{}번째 줄:\r\n\"{}\"",
        // Modes
        "전체 1회",
        "전체 연속",
        "전체 토글",
        "1회",
        "연속",
        "토글"
    },
    // Spanish (Español)
    {
        "KsScript",
        "Gestor de scripts",
        "Abrir scripts",
        "Grabador de scripts",
        "Referencia de funciones",
        "Referencia de teclas",
        "Activar",
        "Nombre del script",
        "Modo de ejecución",
        "Tecla de inicio",
        "Tecla de parada",
        "Desactivar script",
        "Activar script",
        "Detener prueba",
        "Ejecutar prueba",
        "Eliminar script",
        "Recargar",
        "Abrir ubicación del script",
        "Cambiar teclas y modo",
        "Activar todo",
        "Desactivar todo",
        "Eliminar todo",
        "Guardar",
        "Cancelar",
        "Aceptar",
        "Tecla de inicio:",
        "Tecla de parada:",
        "Modo de ejecución:",
        "Grabar",
        "Detener grabación",
        "Ejecutar",
        "Detener ejecución",
        "Guardar script",
        "Estado: GRABANDO...",
        "Estado: EJECUTANDO...",
        "Estado: INACTIVO",
        "Nombre",
        "Parámetros",
        "Explicación",
        "Valor",
        "Error de archivo",
        "Error de sintaxis",
        "Error al guardar",
        "Guardado con éxito",
        "Error al leer el archivo de script UTF-8.\r\n{}",
        "No se pudo leer el archivo para modificarlo",
        "No se pudo abrir el archivo para escritura",
        "Ocurrió un error inesperado al escribir en el archivo",
        "Configuración de script actualizada: {}",
        "El script se guardó en: {}",
        "Coordenada actual",
        // Parser errors
        "Paréntesis '()' faltantes o no válidos",
        "Caracteres tras el paréntesis de cierre ')'. Si es necesario, use # para comentarios.",
        "Formato de tecla de acceso rápido no válido: ",
        "Parámetro de modo de ejecución no válido: ",
        "Parámetro de retardo no válido (debe ser un número entero): ",
        "Parámetro de botón del ratón no válido: ",
        "Parámetros de coordenadas no válidos (deben ser números enteros): ",
        "El comando de movimiento espera exactamente 2 parámetros (x, y): ",
        "Parámetro de tecla o código de tecla virtual no válido: ",
        "El comando de rueda no acepta parámetros: ",
        "Formato de cadena no válido (debe estar entre comillas): ",
        "Formato de cadena no válido (comilla sin cerrar): ",
        "Formato de parámetro no válido: ",
        "El comando de mostrar coordenadas no acepta parámetros: ",
        "Ocurrió una excepción inesperada durante el análisis de la línea.",
        "Función desconocida: ",
        "Error de sintaxis\r\n{}\r\nLínea {}:\r\n\"{}\"",
        "Error de sintaxis\r\nLa tecla de inicio ya está definida\r\nLínea {}:\r\n\"{}\"",
        "Error de sintaxis\r\nLa tecla de parada ya está definida\r\nLínea {}:\r\n\"{}\"",
        "Error de sintaxis\r\nEl modo de ejecución ya está definido\r\nLínea {}:\r\n\"{}\"",
        // Modes
        "Único completo",
        "Continuo completo",
        "Alternancia completa",
        "Único",
        "Continuo",
        "Alternancia"
    },
    // Portuguese (Português)
    {
        "KsScript",
        "Gerenciador de scripts",
        "Abrir scripts",
        "Gravador de scripts",
        "Referência de funções",
        "Referência de teclas",
        "Ativar",
        "Nome do script",
        "Modo de execução",
        "Tecla de início",
        "Tecla de parada",
        "Desativar script",
        "Ativar script",
        "Parar teste",
        "Executar teste",
        "Remover script",
        "Recarregar",
        "Abrir local do script",
        "Alterar teclas e modo",
        "Ativar todos",
        "Desativar todos",
        "Remover todos",
        "Salvar",
        "Cancelar",
        "OK",
        "Tecla de início:",
        "Tecla de parada:",
        "Modo de execução:",
        "Gravar",
        "Parar gravação",
        "Executar",
        "Parar execução",
        "Salvar script",
        "Status: GRAVANDO...",
        "Status: EXECUTANDO...",
        "Status: OCIOSO",
        "Nome",
        "Parâmetros",
        "Explicação",
        "Valor",
        "Erro de arquivo",
        "Erro de sintaxe",
        "Falha ao salvar",
        "Salvo com sucesso",
        "Falha ao ler arquivo de script UTF-8.\r\n{}",
        "Não foi possível ler o arquivo para modificação",
        "Não foi possível abrir o arquivo para gravação",
        "Ocorreu um erro inesperado ao gravar no arquivo",
        "Configurações do script atualizadas: {}",
        "Script salvo em: {}",
        "Coordenada atual",
        // Parser errors
        "Parênteses '()' ausentes ou inválidos",
        "Caracteres após o fechamento ')'. Se necessário, adicione texto após #.",
        "Formato de atalho inválido: ",
        "Parâmetro de modo de execução inválido: ",
        "Parâmetro de atraso inválido (deve ser um número inteiro): ",
        "Parâmetro de botão do mouse inválido: ",
        "Parâmetros de coordenadas inválidos (devem ser inteiros): ",
        "O comando de movimento requer exatamente 2 parâmetros (x, y): ",
        "Parâmetro de tecla ou código virtual inválido: ",
        "O comando de rolagem não aceita parâmetros: ",
        "Formato de texto inválido (deve estar entre aspas): ",
        "Formato de texto inválido (aspas não fechadas): ",
        "Formato de parâmetro inválido: ",
        "O comando de mostrar coordenadas não aceita parâmetros: ",
        "Ocorreu uma exceção inesperada durante o processamento da linha.",
        "Função desconhecida: ",
        "Erro de sintaxe\r\n{}\r\nLinha {}:\r\n\"{}\"",
        "Erro de sintaxe\r\nTecla de início já definida\r\nLinha {}:\r\n\"{}\"",
        "Erro de sintaxe\r\nTecla de parada já definida\r\nLinha {}:\r\n\"{}\"",
        "Erro de sintaxe\r\nModo de execução já definido\r\nLinha {}:\r\n\"{}\"",
        // Modes
        "Único completo",
        "Contínuo completo",
        "Alternância completa",
        "Único",
        "Contínuo",
        "Alternância"
    },
    // French (Français)
    {
        "KsScript",
        "Gestionnaire de scripts",
        "Ouvrir des scripts",
        "Enregistreur de scripts",
        "Référence des fonctions",
        "Référence des touches",
        "Actif",
        "Nom du script",
        "Mode d'exécution",
        "Touche de début",
        "Touche de fin",
        "Désactiver le script",
        "Activer le script",
        "Arrêter le test",
        "Tester",
        "Supprimer le script",
        "Recharger",
        "Ouvrir l'emplacement du script",
        "Modifier les touches et le mode",
        "Tout activer",
        "Tout désactiver",
        "Tout supprimer",
        "Enregistrer",
        "Annuler",
        "OK",
        "Touche de début :",
        "Touche de fin :",
        "Mode d'exécution :",
        "Enregistrer",
        "Arrêter l'enregistrement",
        "Exécuter",
        "Arrêter l'exécution",
        "Enregistrer le script",
        "État : ENREGISTREMENT...",
        "État : EXÉCUTION...",
        "État : INACTIF",
        "Nom",
        "Paramètres",
        "Explication",
        "Valeur",
        "Erreur de fichier",
        "Erreur de syntaxe",
        "Échec de l'enregistrement",
        "Enregistrement réussi",
        "Impossible de lire le fichier de script UTF-8.\r\n{}",
        "Impossible de lire le fichier pour modification",
        "Impossible d'ouvrir le fichier en écriture",
        "Une erreur inattendue est survenue lors de l'écriture dans le fichier",
        "Paramètres du script mis à jour : {}",
        "Script enregistré sous : {}",
        "Coordonnées actuelles",
        // Parser errors
        "Parenthèses '()' manquantes ou invalides",
        "Caractères non autorisés après la parenthèse fermante ')'. Ajoutez les commentaires après #.",
        "Format de raccourci invalide : ",
        "Paramètre de mode d'exécution invalide : ",
        "Paramètre de délai invalide (doit être un entier) : ",
        "Paramètre de bouton de souris invalide : ",
        "Coordonnées invalides (doivent être des entiers) : ",
        "La commande de déplacement attend exactement 2 paramètres (x, y) : ",
        "Touche ou code virtuel invalide : ",
        "La commande de molette n'accepte aucun paramètre : ",
        "Format de chaîne invalide (doit être entre guillemets) : ",
        "Format de chaîne invalide (guillemet non fermé) : ",
        "Format de paramètre invalide : ",
        "La commande d'affichage des coordonnées n'accepte aucun paramètre : ",
        "Une exception inattendue s'est produite lors de l'analyse de la ligne.",
        "Fonction inconnue : ",
        "Erreur de syntaxe\r\n{}\r\nLigne {} :\r\n\"{}\"",
        "Erreur de syntaxe\r\nTouche de début déjà définie\r\nLigne {} :\r\n\"{}\"",
        "Erreur de syntaxe\r\nTouche de fin déjà définie\r\nLigne {} :\r\n\"{}\"",
        "Erreur de syntaxe\r\nMode d'exécution déjà défini\r\nLigne {} :\r\n\"{}\"",
        // Modes
        "Unique complet",
        "Continu complet",
        "Bascule complète",
        "Unique",
        "Continu",
        "Bascule"
    },
    // German (Deutsch)
    {
        "KsScript",
        "Skript-Manager",
        "Skripte öffnen",
        "Skript-Rekorder",
        "Befehlsreferenz",
        "Tastenreferenz",
        "Aktivieren",
        "Skriptname",
        "Ausführungsmodus",
        "Starttaste",
        "Stopptaste",
        "Skript deaktivieren",
        "Skript aktivieren",
        "Testlauf stoppen",
        "Testlauf ausführen",
        "Skript entfernen",
        "Neu laden",
        "Skriptverzeichnis öffnen",
        "Tasten und Modus ändern",
        "Alle aktivieren",
        "Alle deaktivieren",
        "Alle entfernen",
        "Speichern",
        "Abbrechen",
        "OK",
        "Starttaste:",
        "Stopptaste:",
        "Ausführungsmodus:",
        "Aufnehmen",
        "Aufnahme stoppen",
        "Ausführen",
        "Ausführung stoppen",
        "Skript speichern",
        "Status: AUFNAHME...",
        "Status: AUSFÜHRUNG...",
        "Status: BEREIT",
        "Name",
        "Parameter",
        "Beschreibung",
        "Wert",
        "Dateifehler",
        "Skript-Syntaxfehler",
        "Speichern fehlgeschlagen",
        "Erfolgreich gespeichert",
        "Fehler beim Lesen der UTF-8-Skriptdatei.\r\n{}",
        "Datei konnte zur Änderung nicht gelesen werden",
        "Datei konnte nicht zum Schreiben geöffnet werden",
        "Unerwarteter Fehler beim Schreiben in die Datei aufgetreten",
        "Skripteinstellungen aktualisiert: {}",
        "Skript gespeichert unter: {}",
        "Aktuelle Koordinaten",
        // Parser errors
        "Fehlende oder ungültige Klammern '()'",
        "Zeichen nach schließender Klammer ')'. Fügen Sie Kommentare hinter # ein.",
        "Ungültiges Hotkey-Format: ",
        "Ungültiger Ausführungsmodus-Parameter: ",
        "Ungültiger Verzögerungsparameter (muss eine Ganzzahl sein): ",
        "Ungültiger Maustastenparameter: ",
        "Ungültige Koordinatenparameter (müssen Ganzzahlen sein): ",
        "Der Bewegungsbefehl erfordert genau 2 Parameter (x, y): ",
        "Ungültiger Tastenname oder virtueller Tastencode: ",
        "Mausradbefehl akzeptiert keine Parameter: ",
        "Ungültiges Zeichenfolgenformat (muss in Anführungszeichen stehen): ",
        "Ungültiges Zeichenfolgenformat (nicht geschlossene Anführungszeichen): ",
        "Ungültiges Parameterformat: ",
        "Befehl zur Koordinatenanzeige akzeptiert keine Parameter: ",
        "Unerwartete Ausnahme beim Parsen der Zeile aufgetreten.",
        "Unbekannter Befehl: ",
        "Syntaxfehler\r\n{}\r\nZeile {}:\r\n\"{}\"",
        "Syntaxfehler\r\nStarttaste bereits definiert\r\nZeile {}:\r\n\"{}\"",
        "Syntaxfehler\r\nStopptaste bereits definiert\r\nZeile {}:\r\n\"{}\"",
        "Syntaxfehler\r\nAusführungsmodus bereits definiert\r\nZeile {}:\r\n\"{}\"",
        // Modes
        "Vollständig einmalig",
        "Vollständig fortlaufend",
        "Vollständig umschalten",
        "Einmalig",
        "Fortlaufend",
        "Umschalten"
    }
};

static std::vector<FuncInfo> s_funcInfosByLang[static_cast<size_t>(Language::Count)];

static void InitFuncInfos() {
    // English
    s_funcInfosByLang[static_cast<size_t>(Language::English)] = {
        { u8"SetStartKey", u8"(key, [modifiers...])", u8"Defines the script start trigger. Accepts a key name or VK code, with optional modifier states (CtrlDown, CtrlUp, ShiftDown, ShiftUp, AltDown, AltUp)." },
        { u8"SetEndKey", u8"(key, [modifiers...])", u8"Defines the script stop trigger. Accepts a key name or VK code, with optional modifier states (CtrlDown, CtrlUp, ShiftDown, ShiftUp, AltDown, AltUp)." },
        { u8"SetRunMode", u8"(mode)", u8"Sets execution mode. Valid modes: FullSingle, FullContinuous, FullSwitch, Single, Continuous, or Switch." },
        { u8"Delay", u8"(ms)", u8"Pauses script execution for the specified duration in milliseconds." },
        { u8"MouseDown", u8"(buttonName)", u8"Presses a mouse button down. Valid buttons: Left, Right, Middle, X1, X2." },
        { u8"MouseUp", u8"(buttonName)", u8"Releases a mouse button. Valid buttons: Left, Right, Middle, X1, X2." },
        { u8"AbsoluteMove", u8"(x, y)", u8"Moves the mouse cursor to absolute screen coordinates (x, y)." },
        { u8"RelativeMove", u8"(dx, dy)", u8"Moves the mouse cursor relative to its current position by (dx, dy)." },
        { u8"KeyboardDown", u8"(key)", u8"Presses down a keyboard key using a key name or virtual-key (VK) code." },
        { u8"KeyboardUp", u8"(key)", u8"Releases a keyboard key using a key name or virtual-key (VK) code." },
        { u8"WheelDown", u8"()", u8"Scrolls the mouse wheel down. This command takes no arguments." },
        { u8"WheelUp", u8"()", u8"Scrolls the mouse wheel up. This command takes no arguments." },
        { u8"HoldTitleBar", u8"(windowName)", u8"Simulates holding down the title bar of the specified window." },
        { u8"ReleaseTitleBar", u8"(windowName)", u8"Simulates releasing the title bar of the specified window." },
        { u8"OutputText", u8"(text, [delayMs])", u8"Outputs a text string with an optional delay in milliseconds between characters. Single quotes ('...') are literal; double quotes (\"...\") support escape sequences." },
        { u8"ShowCoordinate", u8"()", u8"Displays the current cursor screen coordinates. This command takes no arguments." }
    };

    // Cantonese (Hong Kong) (廣東話)
    s_funcInfosByLang[static_cast<size_t>(Language::Cantonese)] = {
        { u8"設定開始按鍵", u8"(按鍵, [modifiers...])", u8"設定腳本啟動觸發掣。可以用按鍵名稱或按鍵編號，可選modifiers：CtrlDown、CtrlUp、ShiftDown、ShiftUp、AltDown、AltUp。" },
        { u8"設定停止按鍵", u8"(按鍵, [modifiers...])", u8"設定腳本停止觸發掣。可以用按鍵名稱或按鍵編號，可選modifiers：CtrlDown、CtrlUp、ShiftDown、ShiftUp、AltDown、AltUp。" },
        { u8"設定運行模式", u8"(模式)", u8"設定運行模式。有效模式：完整單次、完整連續、完整切換、單次、連續、切換。" },
        { u8"延遲", u8"(延遲毫秒)", u8"將腳本執行暫停指定嘅毫秒數。" },
        { u8"滑鼠按下", u8"(滑鼠掣)", u8"撳落指定嘅滑鼠掣。有效滑鼠掣：左、右、中、X1、X2。" },
        { u8"滑鼠放開", u8"(滑鼠掣)", u8"放開指定嘅滑鼠掣。有效滑鼠掣：左、右、中、X1、X2。" },
        { u8"絕對移動", u8"(x, y)", u8"將滑鼠游標移動到指定嘅絕對螢幕座標 (x, y)。" },
        { u8"相對移動", u8"(dx, dy)", u8"相對於現時位置，按 (dx, dy) 移動滑鼠游標。" },
        { u8"鍵盤按下", u8"(按鍵)", u8"撳落鍵盤按鍵，可以用按鍵名稱或按鍵編號。" },
        { u8"鍵盤放開", u8"(按鍵)", u8"放開鍵盤按鍵，可以用按鍵名稱或按鍵編號。" },
        { u8"滾輪下", u8"()", u8"向下碌滑鼠滾輪。呢個指令唔需要任何引數。" },
        { u8"滾輪上", u8"()", u8"向上碌滑鼠滾輪。呢個指令唔需要任何引數。" },
        { u8"按住視窗標題", u8"(視窗名稱)", u8"模擬撳住指定視窗嘅標題列。" },
        { u8"放開視窗標題", u8"(視窗名稱)", u8"模擬放開指定視窗嘅標題列。" },
        { u8"輸出文字", u8"(文字, [延遲毫秒])", u8"輸出文字，可以選填字符之間嘅延遲毫秒數。單引號 ('...') 表示字面值；雙引號 (\"...\") 支援轉義序列。" },
        { u8"顯示座標", u8"()", u8"顯示現時嘅螢幕滑鼠座標。呢個指令唔需要任何引數。" }
    };

    // Traditional Chinese (繁體中文)
    s_funcInfosByLang[static_cast<size_t>(Language::TraditionalChinese)] = {
        { u8"設定開始按鍵", u8"(按鍵, [modifiers...])", u8"設定腳本啟動觸發鍵。接受按鍵名稱或按鍵編號，可選的modifiers：CtrlDown、CtrlUp、ShiftDown、ShiftUp、AltDown、AltUp。" },
        { u8"設定停止按鍵", u8"(按鍵, [modifiers...])", u8"設定腳本停止觸發鍵。接受按鍵名稱或按鍵編號，可選的modifiers：CtrlDown、CtrlUp、ShiftDown、ShiftUp、AltDown、AltUp。" },
        { u8"設定運行模式", u8"(模式)", u8"設定運行模式。有效模式：完整單次、完整連續、完整切換、單次、連續、切換。" },
        { u8"延遲", u8"(延遲毫秒)", u8"將腳本執行暫停指定的毫秒。" },
        { u8"滑鼠按下", u8"(滑鼠按鍵)", u8"按下指定的滑鼠按鍵。有效滑鼠按鍵：左、右、中、X1、X2。" },
        { u8"滑鼠放開", u8"(滑鼠按鍵)", u8"放開指定的滑鼠按鍵。有效滑鼠按鍵：左、右、中、X1、X2。" },
        { u8"絕對移動", u8"(x, y)", u8"將滑鼠游標移動至指定的絕對螢幕座標 (x, y)。" },
        { u8"相對移動", u8"(dx, dy)", u8"相對於目前位置，依 (dx, dy) 移動滑鼠游標。" },
        { u8"鍵盤按下", u8"(按鍵)", u8"按下鍵盤按鍵，接受按鍵名稱或按鍵編號。" },
        { u8"鍵盤放開", u8"(按鍵)", u8"放開鍵盤按鍵，接受按鍵名稱或按鍵編號。" },
        { u8"滾輪下", u8"()", u8"向下滾動滑鼠滾輪。此指令不需要任何引數。" },
        { u8"滾輪上", u8"()", u8"向上滾動滑鼠滾輪。此指令不需要任何引數。" },
        { u8"按住視窗標題", u8"(視窗名稱)", u8"模擬按住指定視窗的標題列。" },
        { u8"放開視窗標題", u8"(視窗名稱)", u8"模擬放開指定視窗的標題列。" },
        { u8"輸出文字", u8"(文字, [延遲毫秒])", u8"輸出文字，可選填字元間的延遲毫秒數。單引號 ('...') 表示字面值；雙引號 (\"...\") 表示escape sequences。" },
        { u8"顯示座標", u8"()", u8"顯示目前的螢幕滑鼠座標。此指令不需要任何引數。" }
    };

    // Simplified Chinese (简体中文)
    s_funcInfosByLang[static_cast<size_t>(Language::SimplifiedChinese)] = {
        { u8"设定开始按键", u8"(按键, [modifiers...])", u8"设置脚本启动触发键。接受按键名称或按键编号，可选的modifiers：CtrlDown、CtrlUp、ShiftDown、ShiftUp、AltDown、AltUp。" },
        { u8"设定停止按键", u8"(按键, [modifiers...])", u8"设置脚本停止触发键。接受按键名称或按键编号，可选的modifiers：CtrlDown、CtrlUp、ShiftDown、ShiftUp、AltDown、AltUp。" },
        { u8"设定运行模式", u8"(模式)", u8"设置运行模式。有效模式：完整单次、完整连续、完整切换、单次、连续、切换。" },
        { u8"延迟", u8"(延迟毫秒)", u8"将脚本执行暂停指定的毫秒数。" },
        { u8"鼠标按下", u8"(鼠标按键)", u8"按下指定的鼠标按键。有效按键：左、右、中、X1、X2。" },
        { u8"鼠标放开", u8"(鼠标按键)", u8"放开指定的鼠标按键。有效按键：左、右、中、X1、X2。" },
        { u8"绝对移动", u8"(x, y)", u8"将鼠标光标移动至指定的绝对屏幕坐标 (x, y)。" },
        { u8"相对移动", u8"(dx, dy)", u8"相对于当前位置，按 (dx, dy) 移动鼠标光标。" },
        { u8"键盘按下", u8"(按键)", u8"按下键盘按键，接受按键名称或按键编号。" },
        { u8"键盘放开", u8"(按键)", u8"放开键盘按键，接受按键名称或按键编号。" },
        { u8"滚轮下", u8"()", u8"向下滚动鼠标滚轮。此指令不需要任何参数。" },
        { u8"滚轮上", u8"()", u8"向上滚动鼠标滚轮。此指令不需要任何参数。" },
        { u8"按住窗口标题", u8"(窗口名称)", u8"模拟按住指定窗口的标题栏。" },
        { u8"放开窗口标题", u8"(窗口名称)", u8"模拟放开指定窗口的标题栏。" },
        { u8"输出文本", u8"(文本, [延迟毫秒])", u8"输出文本，可选填字符间的延迟毫秒数。单引号 ('...') 表示字面量；双引号 (\"...\") 支持转义序列。" },
        { u8"显示坐标", u8"()", u8"显示当前的屏幕鼠标坐标。此指令不需要任何参数。" }
    };

    // Japanese (日本語)
    s_funcInfosByLang[static_cast<size_t>(Language::Japanese)] = {
        { u8"SetStartKey", u8"(キー, [modifiers...])", u8"スクリプト開始トリガーキーを設定します。キー名または仮想キーコードを指定可能。修飾キー（CtrlDown, CtrlUp, ShiftDown, ShiftUp, AltDown, AltUp）も指定できます。" },
        { u8"SetEndKey", u8"(キー, [modifiers...])", u8"スクリプト停止トリガーキーを設定します。キー名または仮想キーコードを指定可能。修飾キー（CtrlDown, CtrlUp, ShiftDown, ShiftUp, AltDown, AltUp）も指定できます。" },
        { u8"SetRunMode", u8"(モード)", u8"実行モードを設定します。有効なモード：FullSingle, FullContinuous, FullSwitch, Single, Continuous, Switch。" },
        { u8"Delay", u8"(ミリ秒)", u8"スクリプトの実行を指定したミリ秒数だけ一時停止します。" },
        { u8"MouseDown", u8"(ボタン名)", u8"マウスボタンを押した状態にします。有効なボタン：Left, Right, Middle, X1, X2。" },
        { u8"MouseUp", u8"(ボタン名)", u8"マウスボタンを離します。有効なボタン：Left, Right, Middle, X1, X2。" },
        { u8"AbsoluteMove", u8"(x, y)", u8"マウスカーソルを指定された絶対画面座標 (x, y) に移動します。" },
        { u8"RelativeMove", u8"(dx, dy)", u8"現在の位置を基準にして、マウスカーソルを (dx, dy) だけ相対移動します。" },
        { u8"KeyboardDown", u8"(キー)", u8"キーボードのキーを押します。キー名または仮想キー（VK）コードを指定できます。" },
        { u8"KeyboardUp", u8"(キー)", u8"キーボードのキーを離します。キー名または仮想キー（VK）コードを指定できます。" },
        { u8"WheelDown", u8"()", u8"マウスホイールを下方向にスクロールします。引数は不要です。" },
        { u8"WheelUp", u8"()", u8"マウスホイールを上方向にスクロールします。引数は不要です。" },
        { u8"HoldTitleBar", u8"(ウィンドウ名)", u8"指定したウィンドウのタイトルバーを長押しするシミュレーションを行います。" },
        { u8"ReleaseTitleBar", u8"(ウィンドウ名)", u8"指定したウィンドウのタイトルバーを離すシミュレーションを行います。" },
        { u8"OutputText", u8"(テキスト, [遅延ミリ秒])", u8"文字列を出力します。文字間の遅延（ミリ秒）をオプションで指定可能。単一引用符 ('...') はリテラル、二重引用符 (\"...\") はエスケープシーケンスをサポートします。" },
        { u8"ShowCoordinate", u8"()", u8"現在のカーソル画面座標を表示します。引数は不要です。" }
    };

    // Korean (한국어)
    s_funcInfosByLang[static_cast<size_t>(Language::Korean)] = {
        { u8"SetStartKey", u8"(키, [modifiers...])", u8"스크립트 시작 트리거 키를 설정합니다. 키 이름 또는 가상 키 코드를 허용하며, 보조키(CtrlDown, CtrlUp, ShiftDown, ShiftUp, AltDown, AltUp)를 지정할 수 있습니다." },
        { u8"SetEndKey", u8"(키, [modifiers...])", u8"스크립트 정지 트리거 키를 설정합니다. 키 이름 또는 가상 키 코드를 허용하며, 보조키(CtrlDown, CtrlUp, ShiftDown, ShiftUp, AltDown, AltUp)를 지정할 수 있습니다." },
        { u8"SetRunMode", u8"(모드)", u8"실행 모드를 설정합니다. 유효한 모드: FullSingle, FullContinuous, FullSwitch, Single, Continuous, Switch." },
        { u8"Delay", u8"(밀리초)", u8"지정한 시간(밀리초) 동안 스크립트 실행을 일시 중지합니다." },
        { u8"MouseDown", u8"(버튼이름)", u8"마우스 버튼을 누릅니다. 유효한 버튼: Left, Right, Middle, X1, X2." },
        { u8"MouseUp", u8"(버튼이름)", u8"마우스 버튼을 놓습니다. 유효한 버튼: Left, Right, Middle, X1, X2." },
        { u8"AbsoluteMove", u8"(x, y)", u8"마우스 커서를 지정한 절대 화면 좌표 (x, y)로 이동합니다." },
        { u8"RelativeMove", u8"(dx, dy)", u8"현재 위치를 기준으로 마우스 커서를 (dx, dy)만큼 상대 이동합니다." },
        { u8"KeyboardDown", u8"(키)", u8"키보드 키를 누릅니다. 키 이름 또는 가상 키(VK) 코드를 사용합니다." },
        { u8"KeyboardUp", u8"(키)", u8"키보드 키를 놓습니다. 키 이름 또는 가상 키(VK) 코드를 사용합니다." },
        { u8"WheelDown", u8"()", u8"마우스 휠을 아래로 스크롤합니다. 인수가 필요하지 않습니다." },
        { u8"WheelUp", u8"()", u8"마우스 휠을 위로 스크롤합니다. 인수가 필요하지 않습니다." },
        { u8"HoldTitleBar", u8"(창이름)", u8"지정한 창의 제목 표시줄을 길게 누르고 있는 동작을 시뮬레이션합니다." },
        { u8"ReleaseTitleBar", u8"(창이름)", u8"지정한 창의 제목 표시줄을 놓는 동작을 시뮬레이션합니다." },
        { u8"OutputText", u8"(텍스트, [지연밀리초])", u8"텍스트 문자열을 출력합니다. 문자 간 지연(밀리초)을 선택적으로 지정할 수 있습니다. 작은따옴표('...')는 리터럴이며 큰따옴표(\"... \")는 이스케이프 시퀀스를 지원합니다." },
        { u8"ShowCoordinate", u8"()", u8"현재 화면 마우스 좌표를 표시합니다. 인수가 필요하지 않습니다." }
    };

    // Spanish (Español)
    s_funcInfosByLang[static_cast<size_t>(Language::Spanish)] = {
        { u8"SetStartKey", u8"(tecla, [modifiers...])", u8"Define la tecla de inicio del script. Acepta un nombre de tecla o código VK, con modificadores opcionales (CtrlDown, CtrlUp, ShiftDown, ShiftUp, AltDown, AltUp)." },
        { u8"SetEndKey", u8"(tecla, [modifiers...])", u8"Define la tecla de parada del script. Acepta un nombre de tecla o código VK, con modificadores opcionales (CtrlDown, CtrlUp, ShiftDown, ShiftUp, AltDown, AltUp)." },
        { u8"SetRunMode", u8"(modo)", u8"Establece el modo de ejecución. Modos válidos: FullSingle, FullContinuous, FullSwitch, Single, Continuous o Switch." },
        { u8"Delay", u8"(ms)", u8"Pausa la ejecución del script durante la duración especificada en milisegundos." },
        { u8"MouseDown", u8"(botón)", u8"Presiona un botón del ratón. Botones válidos: Left, Right, Middle, X1, X2." },
        { u8"MouseUp", u8"(botón)", u8"Suelta un botón del ratón. Botones válidos: Left, Right, Middle, X1, X2." },
        { u8"AbsoluteMove", u8"(x, y)", u8"Mueve el cursor del ratón a las coordenadas absolutas de la pantalla (x, y)." },
        { u8"RelativeMove", u8"(dx, dy)", u8"Mueve el cursor del ratón en relación con su posición actual en (dx, dy)." },
        { u8"KeyboardDown", u8"(tecla)", u8"Presiona una tecla del teclado usando un nombre de tecla o código de tecla virtual (VK)." },
        { u8"KeyboardUp", u8"(tecla)", u8"Suelta una tecla del teclado usando un nombre de tecla o código de tecla virtual (VK)." },
        { u8"WheelDown", u8"()", u8"Gira la rueda del ratón hacia abajo. Este comando no admite argumentos." },
        { u8"WheelUp", u8"()", u8"Gira la rueda del ratón hacia arriba. Este comando no admite argumentos." },
        { u8"HoldTitleBar", u8"(nombreVentana)", u8"Simula mantener presionada la barra de título de la ventana especificada." },
        { u8"ReleaseTitleBar", u8"(nombreVentana)", u8"Simula soltar la barra de título de la ventana especificada." },
        { u8"OutputText", u8"(texto, [retardoMs])", u8"Escribe una cadena de texto con retraso opcional en milisegundos entre caracteres. Las comillas simples ('...') son literales; las dobles (\"...\") admiten secuencias de escape." },
        { u8"ShowCoordinate", u8"()", u8"Muestra las coordenadas actuales del cursor en pantalla. Este comando no admite argumentos." }
    };

    // Portuguese (Português)
    s_funcInfosByLang[static_cast<size_t>(Language::Portuguese)] = {
        { u8"SetStartKey", u8"(tecla, [modifiers...])", u8"Define a tecla de início do script. Aceita o nome da tecla ou código VK, com modificadores opcionais (CtrlDown, CtrlUp, ShiftDown, ShiftUp, AltDown, AltUp)." },
        { u8"SetEndKey", u8"(tecla, [modifiers...])", u8"Define a tecla de parada do script. Aceita o nome da tecla ou código VK, com modificadores opcionais (CtrlDown, CtrlUp, ShiftDown, ShiftUp, AltDown, AltUp)." },
        { u8"SetRunMode", u8"(modo)", u8"Define o modo de execução. Modos válidos: FullSingle, FullContinuous, FullSwitch, Single, Continuous ou Switch." },
        { u8"Delay", u8"(ms)", u8"Pausa a execução do script pela duração especificada em milissegundos." },
        { u8"MouseDown", u8"(botão)", u8"Pressiona um botão do mouse. Botões válidos: Left, Right, Middle, X1, X2." },
        { u8"MouseUp", u8"(botão)", u8"Solta um botão do mouse. Botões válidos: Left, Right, Middle, X1, X2." },
        { u8"AbsoluteMove", u8"(x, y)", u8"Move o cursor do mouse para as coordenadas absolutas da tela (x, y)." },
        { u8"RelativeMove", u8"(dx, dy)", u8"Move o cursor do mouse relativamente à posição atual em (dx, dy)." },
        { u8"KeyboardDown", u8"(tecla)", u8"Pressiona uma tecla do teclado usando o nome da tecla ou código virtual (VK)." },
        { u8"KeyboardUp", u8"(tecla)", u8"Solta uma tecla do teclado usando o nome da tecla ou código virtual (VK)." },
        { u8"WheelDown", u8"()", u8"Rola a roda do mouse para baixo. Este comando não aceita argumentos." },
        { u8"WheelUp", u8"()", u8"Rola a roda do mouse para cima. Este comando não aceita argumentos." },
        { u8"HoldTitleBar", u8"(nomeJanela)", u8"Simula manter pressionada a barra de título da janela especificada." },
        { u8"ReleaseTitleBar", u8"(nomeJanela)", u8"Simula soltar a barra de título da janela especificada." },
        { u8"OutputText", u8"(texto, [atrasoMs])", u8"Digita uma cadeia de texto com atraso opcional em milissegundos entre caracteres. Aspas simples ('...') são literais; aspas duplas (\"...\") suportam sequências de escape." },
        { u8"ShowCoordinate", u8"()", u8"Exibe as coordenadas atuais do cursor na tela. Este comando não aceita argumentos." }
    };

    // French (Français)
    s_funcInfosByLang[static_cast<size_t>(Language::French)] = {
        { u8"SetStartKey", u8"(touche, [modifiers...])", u8"Définit la touche de déclenchement du script. Accepte un nom de touche ou code VK, avec modificateurs optionnels (CtrlDown, CtrlUp, ShiftDown, ShiftUp, AltDown, AltUp)." },
        { u8"SetEndKey", u8"(touche, [modifiers...])", u8"Définit la touche d'arrêt du script. Accepte un nom de touche ou code VK, avec modificateurs optionnels (CtrlDown, CtrlUp, ShiftDown, ShiftUp, AltDown, AltUp)." },
        { u8"SetRunMode", u8"(mode)", u8"Définit le mode d'exécution. Modes valides : FullSingle, FullContinuous, FullSwitch, Single, Continuous ou Switch." },
        { u8"Delay", u8"(ms)", u8"Suspend l'exécution du script pendant la durée spécifiée en millisecondes." },
        { u8"MouseDown", u8"(bouton)", u8"Maintient un bouton de la souris enfoncé. Boutons valides : Left, Right, Middle, X1, X2." },
        { u8"MouseUp", u8"(bouton)", u8"Relâche un bouton de la souris. Boutons valides : Left, Right, Middle, X1, X2." },
        { u8"AbsoluteMove", u8"(x, y)", u8"Déplace le curseur de la souris aux coordonnées absolues de l'écran (x, y)." },
        { u8"RelativeMove", u8"(dx, dy)", u8"Déplace le curseur de la souris de (dx, dy) par rapport à sa position actuelle." },
        { u8"KeyboardDown", u8"(touche)", u8"Enfonce une touche du clavier en utilisant un nom de touche ou code virtuel (VK)." },
        { u8"KeyboardUp", u8"(touche)", u8"Relâche une touche du clavier en utilisant un nom de touche ou code virtuel (VK)." },
        { u8"WheelDown", u8"()", u8"Fait défiler la molette de la souris vers le bas. Ne prend aucun argument." },
        { u8"WheelUp", u8"()", u8"Fait défiler la molette de la souris vers le haut. Ne prend aucun argument." },
        { u8"HoldTitleBar", u8"(nomFenêtre)", u8"Simule le maintien enfoncé de la barre de titre de la fenêtre spécifiée." },
        { u8"ReleaseTitleBar", u8"(nomFenêtre)", u8"Simule le relâchement de la barre de titre de la fenêtre spécifiée." },
        { u8"OutputText", u8"(texte, [délaiMs])", u8"Saisit une chaîne de texte avec un délai optionnel en millisecondes entre les caractères. Les guillemets simples ('...') sont littéraux ; les guillemets doubles (\"...\") gèrent les séquences d'échappement." },
        { u8"ShowCoordinate", u8"()", u8"Affiche les coordonnées actuelles du curseur à l'écran. Ne prend aucun argument." }
    };

    // German (Deutsch)
    s_funcInfosByLang[static_cast<size_t>(Language::German)] = {
        { u8"SetStartKey", u8"(Taste, [modifiers...])", u8"Legt die Starttaste des Skripts fest. Akzeptiert einen Tastennamen oder VK-Code mit optionalen Modifikatoren (CtrlDown, CtrlUp, ShiftDown, ShiftUp, AltDown, AltUp)." },
        { u8"SetEndKey", u8"(Taste, [modifiers...])", u8"Legt die Stopptaste des Skripts fest. Akzeptiert einen Tastennamen oder VK-Code mit optionalen Modifikatoren (CtrlDown, CtrlUp, ShiftDown, ShiftUp, AltDown, AltUp)." },
        { u8"SetRunMode", u8"(Modus)", u8"Legt den Ausführungsmodus fest. Gültige Modi: FullSingle, FullContinuous, FullSwitch, Single, Continuous oder Switch." },
        { u8"Delay", u8"(ms)", u8"Pausiert die Skriptausführung für die angegebene Dauer in Millisekunden." },
        { u8"MouseDown", u8"(Taste)", u8"Drückt eine Maustaste herunter. Gültige Tasten: Left, Right, Middle, X1, X2." },
        { u8"MouseUp", u8"(Taste)", u8"Lässt eine Maustaste los. Gültige Tasten: Left, Right, Middle, X1, X2." },
        { u8"AbsoluteMove", u8"(x, y)", u8"Bewegt den Mauszeiger zu den absoluten Bildschirmkoordinaten (x, y)." },
        { u8"RelativeMove", u8"(dx, dy)", u8"Bewegt den Mauszeiger relativ zu seiner aktuellen Position um (dx, dy)." },
        { u8"KeyboardDown", u8"(Taste)", u8"Drückt eine Tastaturtaste herunter (unter Verwendung eines Tastennamens oder VK-Codes)." },
        { u8"KeyboardUp", u8"(Taste)", u8"Lässt eine Tastaturtaste los (unter Verwendung eines Tastennamens oder VK-Codes)." },
        { u8"WheelDown", u8"()", u8"Dreht das Mausrad nach unten. Dieser Befehl akzeptiert keine Parameter." },
        { u8"WheelUp", u8"()", u8"Dreht das Mausrad nach oben. Dieser Befehl akzeptiert keine Parameter." },
        { u8"HoldTitleBar", u8"(Fenstername)", u8"Simuliert das Gedrückthalten der Titelleiste des angegebenen Fensters." },
        { u8"ReleaseTitleBar", u8"(Fenstername)", u8"Simuliert das Loslassen der Titelleiste des angegebenen Fensters." },
        { u8"OutputText", u8"(Text, [VerzögerungMs])", u8"Gibt eine Textzeichenfolge mit optionaler Verzögerung in Millisekunden zwischen den Zeichen aus. Einfache Anführungszeichen ('...') sind wörtlich; doppelte Anführungszeichen (\"...\") unterstützen Escape-Sequenzen." },
        { u8"ShowCoordinate", u8"()", u8"Zeigt die aktuellen Bildschirmkoordinaten des Zeigers an. Dieser Befehl akzeptiert keine Parameter." }
    };
}

static std::filesystem::path GetConfigPath() {
    wchar_t exePathBuffer[MAX_PATH];
    DWORD len = ::GetModuleFileNameW(nullptr, exePathBuffer, MAX_PATH);
    if (len > 0) {
        return std::filesystem::path(exePathBuffer).parent_path() / "config.ini";
    }
    return std::filesystem::current_path() / "config.ini";
}

static Language DetectSystemLanguage() {
    LANGID langId = GetUserDefaultUILanguage();
    WORD primaryLang = PRIMARYLANGID(langId);
    WORD subLang = SUBLANGID(langId);

    if (primaryLang == LANG_CHINESE) {
        if (subLang == SUBLANG_CHINESE_HONGKONG || subLang == SUBLANG_CHINESE_MACAU) {
            return Language::Cantonese;
        } else if (subLang == SUBLANG_CHINESE_TRADITIONAL) {
            return Language::TraditionalChinese;
        } else {
            return Language::SimplifiedChinese;
        }
    }

    switch (primaryLang) {
    case LANG_JAPANESE:
        return Language::Japanese;
    case LANG_KOREAN:
        return Language::Korean;
    case LANG_SPANISH:
        return Language::Spanish;
    case LANG_PORTUGUESE:
        return Language::Portuguese;
    case LANG_FRENCH:
        return Language::French;
    case LANG_GERMAN:
        return Language::German;
    default:
        return Language::English;
    }
}

void I18n::Init() {
    if (!s_initialized) {
        InitFuncInfos();
        LoadConfig();
        s_initialized = true;
    }
}

Language I18n::GetLanguage() {
    if (!s_initialized) {
        Init();
    }
    return s_currentLanguage;
}

void I18n::SetLanguage(Language lang) {
    if (static_cast<size_t>(lang) < static_cast<size_t>(Language::Count)) {
        s_currentLanguage = lang;
    }
}

const char* I18n::Get(StringId id) {
    if (!s_initialized) {
        Init();
    }
    size_t langIdx = static_cast<size_t>(s_currentLanguage);
    size_t strIdx = static_cast<size_t>(id);
    if (langIdx < static_cast<size_t>(Language::Count) && strIdx < static_cast<size_t>(StringId::Count)) {
        return s_translations[langIdx][strIdx];
    }
    return "";
}

std::u8string I18n::GetU8(StringId id) {
    const char* str = Get(id);
    return std::u8string(reinterpret_cast<const char8_t*>(str));
}

std::wstring I18n::GetW(StringId id) {
    const char* str = Get(id);
    if (!str || !*str) return {};

    const int size = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
    if (size <= 0) {
        return {};
    }

    std::wstring result(size, L'\0');
    if (!MultiByteToWideChar(CP_UTF8, 0, str, -1, result.data(), size)) {
        return {};
    }
    result.pop_back();
    return result;
}

const char* const* I18n::GetLanguageNames() {
    return s_languageNames;
}

const std::vector<FuncInfo>& I18n::GetFuncInfos() {
    if (!s_initialized) {
        Init();
    }
    size_t langIdx = static_cast<size_t>(s_currentLanguage);
    if (langIdx < static_cast<size_t>(Language::Count)) {
        return s_funcInfosByLang[langIdx];
    }
    return s_funcInfosByLang[0];
}

const char* I18n::GetRunModeString(int modeIndex) {
    if (!s_initialized) {
        Init();
    }
    size_t langIdx = static_cast<size_t>(s_currentLanguage);
    StringId modeIds[] = {
        StringId::ModeFullSingle,
        StringId::ModeFullContinuous,
        StringId::ModeFullSwitch,
        StringId::ModeSingle,
        StringId::ModeContinuous,
        StringId::ModeSwitch
    };
    if (modeIndex >= 0 && modeIndex < 6 && langIdx < static_cast<size_t>(Language::Count)) {
        return s_translations[langIdx][static_cast<size_t>(modeIds[modeIndex])];
    }
    return "";
}

void I18n::LoadConfig() {
    auto configPath = GetConfigPath();
    if (std::filesystem::exists(configPath)) {
        std::ifstream file(configPath);
        std::string line;
        while (std::getline(file, line)) {
            size_t eqPos = line.find('=');
            if (eqPos != std::string::npos) {
                std::string key = line.substr(0, eqPos);
                std::string val = line.substr(eqPos + 1);
                // trim
                key.erase(0, key.find_first_not_of(" \t\r\n"));
                key.erase(key.find_last_not_of(" \t\r\n") + 1);
                val.erase(0, val.find_first_not_of(" \t\r\n"));
                val.erase(val.find_last_not_of(" \t\r\n") + 1);

                if (key == "Language") {
                    for (size_t i = 0; i < static_cast<size_t>(Language::Count); ++i) {
                        if (val == s_languageCodes[i] || val == s_languageNames[i]) {
                            s_currentLanguage = static_cast<Language>(i);
                            return;
                        }
                    }
                    // Try parsing as integer index
                    try {
                        int idx = std::stoi(val);
                        if (idx >= 0 && idx < static_cast<int>(Language::Count)) {
                            s_currentLanguage = static_cast<Language>(idx);
                            return;
                        }
                    } catch (...) {}
                }
            }
        }
    }

    // Default: detect system language
    s_currentLanguage = DetectSystemLanguage();
}

void I18n::SaveConfig() {
    try {
        auto configPath = GetConfigPath();
        std::ofstream file(configPath);
        if (file.is_open()) {
            file << "[Settings]\n";
            file << "Language=" << s_languageCodes[static_cast<size_t>(s_currentLanguage)] << "\n";
        }
    } catch (...) {}
}

std::string WideToUtf8(std::wstring_view value) {
    if (value.empty()) return {};
    if (value.size() > static_cast<size_t>((std::numeric_limits<int>::max)())) throw std::length_error("Wide string is too long");

    const int inputSize = static_cast<int>(value.size());
    const int size = WideCharToMultiByte(CP_UTF8, 0, value.data(), inputSize, nullptr, 0, nullptr, nullptr);
    if (size == 0) {
        throw std::runtime_error("WideCharToMultiByte failed");
    }

    std::string result(size, '\0');
    if (WideCharToMultiByte(CP_UTF8, 0, value.data(), inputSize, result.data(), size, nullptr, nullptr) == 0) {
        throw std::runtime_error("WideCharToMultiByte failed");
    }
    return result;
}

std::u8string WideToUtf8U8(std::wstring_view value) {
    if (value.empty()) return {};
    if (value.size() > static_cast<size_t>((std::numeric_limits<int>::max)())) throw std::length_error("Wide string is too long");

    const int inputSize = static_cast<int>(value.size());
    const int size = WideCharToMultiByte(CP_UTF8, 0, value.data(), inputSize, nullptr, 0, nullptr, nullptr);
    if (size == 0) {
        throw std::runtime_error("WideCharToMultiByte failed");
    }

    std::u8string result(size, '\0');
    if (WideCharToMultiByte(CP_UTF8, 0, value.data(), inputSize, reinterpret_cast<char*>(result.data()), size, nullptr, nullptr) == 0) {
        throw std::runtime_error("WideCharToMultiByte failed");
    }
    return result;
}

std::wstring Utf8ToWide(std::string_view value) {
    if (value.empty()) return {};
    if (value.size() > static_cast<size_t>((std::numeric_limits<int>::max)())) throw std::length_error("string is too long");

    const int inputSize = static_cast<int>(value.size());
    const int size = MultiByteToWideChar(CP_UTF8, 0, value.data(), inputSize, nullptr, 0);
    if (size == 0) {
        throw std::runtime_error("MultiByteToWideChar failed");
    }

    std::wstring result(size, L'\0');
    if (MultiByteToWideChar(CP_UTF8, 0, value.data(), inputSize, result.data(), size) == 0) {
        throw std::runtime_error("MultiByteToWideChar failed");
    }
    return result;
}