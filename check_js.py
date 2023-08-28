import os
import csv

def search_js_files(directory, keyword):
    matches = []
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith(".js"):
                with open(os.path.join(root, file), 'r', encoding='utf-8', errors='ignore') as js_file:
                    lines = js_file.readlines()
                    for line in lines:
                        if keyword in line:
                            matches.append((os.path.join(root, file), keyword))
    return matches

def record_findings_in_csv(filename, findings):
    with open(filename, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(["JS File", "Matched Line from Source"])
        for finding in findings:
            writer.writerow([finding[0], finding[1]])

def main():
    source_file = 'RemovedApis.txt'
    search_directory = '../overte-content'
    output_csv = 'overte-content-findings.csv'

    findings = []

    with open(source_file, 'r') as f:
        lines = f.readlines()
        for line in lines:
            line = line.strip()
            matches = search_js_files(search_directory, line)
            findings.extend(matches)

    record_findings_in_csv(output_csv, findings)
    print(f"Results written to {output_csv}")

if __name__ == "__main__":
    main()
