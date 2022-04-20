import numpy as np
from sklearn.metrics import r2_score  # type: ignore
from sklearn.model_selection import KFold  # type: ignore


def oof(reg, bundle, folds=5, seed=42, assay=""):
    """Calculate out-of-fold errors of a given model.

    Args:
        reg : Regression model that should support fit/predict methods
        bundle (Tuple): contains train and test inputs and predictions

    Returns:
        Tuple: return list of test errors and list of predictions
    """
    kf = KFold(n_splits=folds, shuffle=True, random_state=seed)
    X_train, X_test, y_train, y_test = bundle

    y_train = y_train[assay].values
    y_test = y_test[assay].values
    train_errors = []
    oof_errors = []
    errors = []

    for i, (train_idx, test_idx) in enumerate(kf.split(X_train, y_train)):
        X_tr = X_train[train_idx]
        y_tr = y_train[train_idx]
        X_te = X_train[test_idx]

        reg.fit(X_tr, y_tr)

        # predict values given validation, train and test sets
        oof_pred = reg.predict(X_te)
        train_pred = reg.predict(X_tr)
        test_pred = reg.predict(X_test)
        # calculate r2 scores
        oof_score = r2_score(y_train[test_idx], oof_pred)
        train_score = r2_score(y_tr, train_pred)
        test_score = r2_score(y_test, test_pred)
        train_errors.append(train_score)
        oof_errors.append(oof_score)
        errors.append(test_score)

    print("MEAN TRAIN: ", np.mean(train_errors))
    print("MEAN VALIDATION: ", np.mean(oof_errors))
    print("MEAN TEST", np.mean(errors))
    return errors, test_pred
