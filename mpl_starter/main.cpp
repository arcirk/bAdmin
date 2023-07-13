#include <QCoreApplication>
#include <QStringList>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QFileInfo>
#include <QProcess>

#define PROFILE_MANAGER_NAME "ProfileManager"

void copy_all(const QString& repo_dir, const QString& dest){
    QDir dir(repo_dir);
    if (!dir.exists())
        return;

    foreach (QString d, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QString dst_path = dest + QDir::separator() + d;
        dir.mkpath(dst_path);
        copy_all(repo_dir+ QDir::separator() + d, dst_path);
    }

    // Счетчик
    //int current = 1;
    auto list = dir.entryList(QDir::Files);
    foreach (QString f, list) {
        QString s = repo_dir + QDir::separator() + f;
        QString d = dest + QDir::separator() + f;
        QFile::copy(s, d);
    }
}

bool verify_repo(const QString& repo_dir, QString& mpl){

    QDir repo(repo_dir);
    if(!repo.exists())
        return false;

    auto app_home = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString mpl_dir = app_home.replace("mpl_starter", PROFILE_MANAGER_NAME);
    mpl_dir = mpl_dir + QDir::separator() + "bin";

    QDir dest(mpl_dir);

    QString file_name = PROFILE_MANAGER_NAME;

#ifdef Q_OS_WINDOWS
    file_name.append(".exe");
#endif

    bool c_all = true;
    if(!dest.exists()){
        auto b = dest.mkpath(mpl_dir);
        if(!b)
            return false;
    }else{
        c_all = false;
        auto repo_file = QFileInfo(repo_dir + QDir::separator() + file_name);
        auto mpl_file = QFileInfo(mpl_dir + QDir::separator() + file_name);
        mpl = mpl_file.absoluteFilePath();
        if(mpl_file.exists() && repo_file.exists()){
            if(repo_file.lastModified() > mpl_file.lastModified()){
                QFile f(mpl_file.absoluteFilePath());
                auto result = f.remove();
                if(!result)
                    return false;
                QFile d(repo_file.absoluteFilePath());
                result = d.copy(mpl_file.absoluteFilePath());
                if(result)
                    mpl = mpl_file.absoluteFilePath();
                return result;
            }else
                return true;
        }else{
            return mpl_file.exists();
        }
    }

    if(c_all){
        copy_all(repo_dir, mpl_dir);
        QFile d(mpl_dir + QDir::separator() + file_name);
        mpl = mpl_dir + QDir::separator() + file_name;
        return d.exists();
    }

    return false;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QStringList argumentList = a.arguments();

    if(argumentList.size() != 2)
        return 0;

    auto repo_dir = argumentList[1];
    QString mpl_file;
    auto result = verify_repo(repo_dir, mpl_file);
    QFileInfo f(mpl_file);
    if(result){
        //start process
        QProcess process;
        process.setProgram(mpl_file);
        process.setWorkingDirectory(f.absolutePath());
        process.startDetached();
    }

    return 0; // a.exec();
}
